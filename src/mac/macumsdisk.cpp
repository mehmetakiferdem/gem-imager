/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include "macumsdisk.h"
#include <QDebug>
#include <QProcess>
#include <QThread>

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

/* Someone else - the pre-write unmount, Finder - asked for the disk. Keep it:
   the write needs it, and eject() releases the claims itself. */
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

namespace {
struct Request {
    dispatch_semaphore_t done;
    QString result;
    bool ok = false;
};

void requestDone(DADiskRef, DADissenterRef dissenter, void *context)
{
    auto req = static_cast<Request *>(context);
    req->ok = !dissenter;
    req->result = daStatus(dissenter);
    dispatch_semaphore_signal(req->done);
}

/* Wait for a Disk Arbitration callback; false if it never came */
bool waitFor(Request &req)
{
    return dispatch_semaphore_wait(req.done, dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC)) == 0;
}
}

bool MacUmsDisk::ejectOnce(const char *bsdName)
{
    if (!_session)
        return false;
    DADiskRef disk = DADiskCreateFromBSDName(kCFAllocatorDefault, _session, bsdName);
    if (!disk) {
        qDebug() << "Disk Arbitration:" << bsdName << "not found";
        return false;
    }

    Request req;
    req.done = dispatch_semaphore_create(0);

    DADiskUnmount(disk, kDADiskUnmountOptionWhole | kDADiskUnmountOptionForce, requestDone, &req);
    if (waitFor(req))
        qDebug() << "Disk Arbitration: unmounting" << bsdName << req.result;
    else
        qDebug() << "Disk Arbitration: no answer to unmounting" << bsdName;

    DADiskEject(disk, kDADiskEjectOptionDefault, requestDone, &req);
    bool ok = false;
    if (waitFor(req)) {
        qDebug() << "Disk Arbitration: ejecting" << bsdName << req.result;
        ok = req.ok;
    } else {
        qDebug() << "Disk Arbitration: no answer to ejecting" << bsdName;
    }

    dispatch_release(req.done);
    CFRelease(disk);
    return ok;
}

bool MacUmsDisk::eject(const QByteArray &device)
{
    releaseClaims();

    QByteArray bsdName = device;
    if (bsdName.startsWith("/dev/"))
        bsdName.remove(0, 5);
    if (bsdName.startsWith("rdisk"))
        bsdName.remove(0, 1);

    /* Closing the disk after writing makes macOS read the new partition
       table, and the disk may be briefly unknown to it while it does */
    for (int attempt = 1; attempt <= 3; attempt++) {
        if (ejectOnce(bsdName.constData()))
            return true;
        QThread::sleep(1);
    }

    QProcess diskutil;
    diskutil.start("diskutil", {"eject", "/dev/" + QString::fromLatin1(bsdName)});
    diskutil.waitForFinished(30000);
    qDebug() << "diskutil eject:" << diskutil.exitCode() << diskutil.readAll().trimmed();
    return diskutil.exitStatus() == QProcess::NormalExit && diskutil.exitCode() == 0;
}
