#ifndef DFUDRIVER_H
#define DFUDRIVER_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include <QString>
#include <QtGlobal>

/*
 * On Windows libusb can only reach a USB device once one of the drivers it
 * supports is bound to that device, and nothing binds one to the board's ROM
 * DFU interface by default. Until the user does it by hand (today: with
 * Zadig) every transfer fails with an opaque libusb error that says nothing
 * about the real cause. These helpers report the driver state so the UI can
 * explain what is actually wrong.
 *
 * Linux gates access through udev rules and macOS needs no driver at all, so
 * on those platforms there is no per-device driver state to report and probe()
 * answers Unknown.
 */
namespace DfuDriver
{
    enum State {
        Unknown,      /* platform does not bind drivers per device */
        NoDevice,     /* no device with this VID/PID is attached */
        NeedsDriver,  /* attached, but no driver libusb can use is bound */
        Ready,        /* attached, with a driver libusb can use */
    };

    /* When boundDriver is given it receives the name of the driver service
       bound to the device ("WinUSB", "usbccgp", ...), or an empty string if
       nothing is bound. */
    State probe(quint16 vendorId, quint16 productId, QString *boundDriver = nullptr);

    /* Explanation to show when a transfer failed, or a null string when a
       missing driver is not the cause. */
    QString missingDriverHint(quint16 vendorId, quint16 productId);
}

#endif // DFUDRIVER_H
