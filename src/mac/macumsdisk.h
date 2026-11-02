#ifndef MACUMSDISK_H
#define MACUMSDISK_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include <QList>
#include <QString>
#include <DiskArbitration/DiskArbitration.h>
#include <dispatch/dispatch.h>

/*
 * Keeps macOS away from U-Boot's ums disk while the image is written.
 *
 * U-Boot clears the partition table before it exposes the eMMC, so macOS
 * finds nothing it can mount and asks whether to initialize, ignore or eject
 * the disk - and ejecting from that dialog stops the write. A claimed disk is
 * left alone: it is not mounted and gets no dialog. Disks are claimed from a
 * Disk Arbitration peek callback, before anything else sees them, which is
 * why this has to exist before the board is switched to mass storage mode.
 * The partitions the image brings along appear once the disk is closed after
 * writing, and are claimed the same way; holding them until the board leaves
 * USB keeps them from being mounted in the meantime.
 */
class MacUmsDisk
{
public:
    MacUmsDisk();
    ~MacUmsDisk();   /* releases the claims; keep it until the board has left */

    /*
     * U-Boot leaves mass storage mode, and writes the boot binaries, on a SCSI
     * eject, and macOS never sends one: it marks every USB disk "Manual
     * Eject" and only detaches it on its own side. Instead, write the string
     * U-Boot takes for "done" to the last sector of the open disk.
     */
    static bool tellBoardWriteIsDone(int fd);

private:
    static void peek(DADiskRef disk, void *context);
    static DADissenterRef release(DADiskRef disk, void *context);
    static void claimed(DADiskRef disk, DADissenterRef dissenter, void *context);
    static void releaseOnQueue(void *context);
    void releaseClaims();

    DASessionRef _session = nullptr;
    dispatch_queue_t _queue = nullptr;
    /* Only touched on _queue */
    bool _peeking = false;
    QList<DADiskRef> _claimed;   /* retained */
};

#endif // MACUMSDISK_H
