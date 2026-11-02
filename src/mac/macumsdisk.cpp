/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include "macumsdisk.h"
#include <QDebug>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/disk.h>
#include <sys/ioctl.h>

/* U-Boot's inquiry data: vendor "Linux", product "UMS disk 0". Partitions
   carry their own media name but share the device's vendor and model. */
static bool isUmsDisk(DADiskRef disk)
{
    CFDictionaryRef desc = DADiskCopyDescription(disk);
    if (!desc)
        return false;
    auto str = [desc](const void *key) {
        CFTypeRef v = CFDictionaryGetValue(desc, key);
        return (v && CFGetTypeID(v) == CFStringGetTypeID())
            ? QString::fromCFString(static_cast<CFStringRef>(v)).trimmed() : QString();
    };
    const QString vendor = str(kDADiskDescriptionDeviceVendorKey);
    const QString model = str(kDADiskDescriptionDeviceModelKey);
    const QString media = str(kDADiskDescriptionMediaNameKey);
    CFRelease(desc);

    const bool match = (vendor == QLatin1String("Linux") && model.startsWith(QLatin1String("UMS disk")))
        || media.contains(QLatin1String("UMS disk"));
    if (match)
        qDebug() << "Disk Arbitration: board disk" << DADiskGetBSDName(disk)
                 << "vendor:" << vendor << "model:" << model << "media:" << media;
    return match;
}

static QString daStatus(DADissenterRef dissenter)
{
    if (!dissenter)
        return QStringLiteral("ok");
    QString s = QString("0x%1").arg(quint32(DADissenterGetStatus(dissenter)), 8, 16, QLatin1Char('0'));
    CFStringRef why = DADissenterGetStatusString(dissenter);
    if (why)
        s += " " + QString::fromCFString(why);
    return s;
}

MacUmsDisk::MacUmsDisk()
{
    _queue = dispatch_queue_create("org.t3gemstone.imager.umsdisk", DISPATCH_QUEUE_SERIAL);
    _session = DASessionCreate(kCFAllocatorDefault);
    if (!_session) {
        qDebug() << "Disk Arbitration: could not create a session; the board's disk will not be claimed";
        return;
    }
    DARegisterDiskPeekCallback(_session, nullptr, 0, peek, this);
    _peeking = true;
    DASessionSetDispatchQueue(_session, _queue);
}

MacUmsDisk::~MacUmsDisk()
{
    releaseClaims();
    if (_session) {
        DASessionSetDispatchQueue(_session, nullptr);
        CFRelease(_session);
    }
    dispatch_release(_queue);
}

void MacUmsDisk::peek(DADiskRef disk, void *context)
{
    if (!isUmsDisk(disk))
        return;
    DADiskClaim(disk, kDADiskClaimOptionDefault, release, context, claimed, context);
}

/* Someone else - the pre-write unmount, Finder - asked for the disk. Keep it
   until the board has left; the claims go with this object. */
DADissenterRef MacUmsDisk::release(DADiskRef, void *)
{
    return DADissenterCreate(kCFAllocatorDefault, kDAReturnBusy, CFSTR("Gemstone Imager is writing to this disk"));
}

void MacUmsDisk::claimed(DADiskRef disk, DADissenterRef dissenter, void *context)
{
    qDebug() << "Disk Arbitration: claiming" << DADiskGetBSDName(disk) << daStatus(dissenter);
    if (dissenter)
        return;
    auto self = static_cast<MacUmsDisk *>(context);
    CFRetain(disk);
    self->_claimed.append(disk);
}

void MacUmsDisk::releaseOnQueue(void *context)
{
    auto self = static_cast<MacUmsDisk *>(context);
    if (self->_peeking) {
        DAUnregisterCallback(self->_session, reinterpret_cast<void *>(peek), self);
        self->_peeking = false;
    }
    for (DADiskRef disk : self->_claimed) {
        DADiskUnclaim(disk);
        CFRelease(disk);
    }
    self->_claimed.clear();
}

void MacUmsDisk::releaseClaims()
{
    dispatch_sync_f(_queue, this, releaseOnQueue);
}

/* Must match FSG_HOST_DONE_MAGIC in U-Boot's f_mass_storage.c */
static const char HOST_DONE[] = "U-Boot UMS: host is done, leave mass storage mode";

bool MacUmsDisk::tellBoardWriteIsDone(int fd)
{
    uint64_t count = 0;
    uint32_t size = 0;
    if (::ioctl(fd, DKIOCGETBLOCKCOUNT, &count) != 0 || ::ioctl(fd, DKIOCGETBLOCKSIZE, &size) != 0
        || count == 0 || size < sizeof(HOST_DONE)) {
        qDebug() << "Could not get the size of the board's disk:" << strerror(errno);
        return false;
    }

    void *sector = nullptr;
    if (posix_memalign(&sector, 4096, size) != 0)
        return false;
    memset(sector, 0, size);
    memcpy(sector, HOST_DONE, sizeof(HOST_DONE));
    const bool ok = ::pwrite(fd, sector, size, off_t((count - 1) * size)) == ssize_t(size)
        && ::fsync(fd) == 0;
    if (!ok)
        qDebug() << "Telling the board the write is done failed:" << strerror(errno);
    else
        qDebug() << "Told the board the write is done (sector" << count - 1 << ")";
    free(sector);
    return ok;
}

namespace {
struct Request {
    dispatch_semaphore_t done;
    QString result;
};

void requestDone(DADiskRef, DADissenterRef dissenter, void *context)
{
    auto req = static_cast<Request *>(context);
    req->result = daStatus(dissenter);
    dispatch_semaphore_signal(req->done);
}

QString waitFor(Request &req)
{
    if (dispatch_semaphore_wait(req.done, dispatch_time(DISPATCH_TIME_NOW, 10 * NSEC_PER_SEC)) != 0)
        return QStringLiteral("no answer");
    return req.result;
}
}

void MacUmsDisk::detach(const QByteArray &device)
{
    releaseClaims();
    if (!_session)
        return;

    QByteArray bsdName = device;
    if (bsdName.startsWith("/dev/"))
        bsdName.remove(0, 5);
    if (bsdName.startsWith("rdisk"))
        bsdName.remove(0, 1);

    DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault, _session, bsdName.constData());
    if (!disk) {
        qDebug() << "Disk Arbitration:" << bsdName << "not found";
        return;
    }

    /* The eject is sent whether or not the unmount succeeded: with nothing
       mounted, macOS refuses the unmount */
    Request req;
    req.done = dispatch_semaphore_create(0);
    DADiskUnmount(disk, kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce, requestDone, &req);
    qDebug() << "Disk Arbitration: unmounting" << bsdName << waitFor(req);
    DADiskEject(disk, kDADiskEjectOptionDefault, requestDone, &req);
    qDebug() << "Disk Arbitration: detaching" << bsdName << waitFor(req);
    dispatch_release(req.done);
    CFRelease(disk);
}
