/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020 Raspberry Pi Ltd
 */

#include "powersaveblocker.h"
#include <QDebug>
#include <string>

#ifdef Q_OS_LINUX
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <unistd.h>
#endif

PowerSaveBlocker::PowerSaveBlocker(QObject *parent) :
    QObject(parent), _stayingAwake(false)
{
}

PowerSaveBlocker::~PowerSaveBlocker()
{
    if (_stayingAwake)
        removeBlock();
}

/*
 * Keeps the computer from going to sleep while an image is written. A sleep
 * in the middle of a write leaves the card or the board half written; with
 * a board it also stops Gemstone Imager from noticing when the board has
 * finished, so the write seemed to hang until the computer woke up.
 */
void PowerSaveBlocker::applyBlock(const QString &reason)
{
    if (_stayingAwake)
        return;

#ifdef Q_OS_WIN
    REASON_CONTEXT rc;
    std::wstring wreason = reason.toStdWString();
    rc.Version = POWER_REQUEST_CONTEXT_VERSION;
    rc.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    rc.Reason.SimpleReasonString = (wchar_t *) wreason.c_str();
    _powerRequest = PowerCreateRequest(&rc);

    if (_powerRequest == INVALID_HANDLE_VALUE)
    {
        qDebug() << "Error creating power request:" << GetLastError();
        return;
    }

    _stayingAwake = PowerSetRequest(_powerRequest, PowerRequestDisplayRequired);
    if (!_stayingAwake)
    {
        qDebug() << "Error running PowerSetRequest():" << GetLastError();
        CloseHandle(_powerRequest);
    }
#endif

#ifdef Q_OS_DARWIN
    /* Idle sleep only: closing the lid still sleeps, as the user asked for it */
    CFStringRef cfReason = reason.toCFString();
    IOReturn ret = IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleSystemSleep,
                                               kIOPMAssertionLevelOn, cfReason, &_assertion);
    CFRelease(cfReason);
    _stayingAwake = (ret == kIOReturnSuccess);
    if (!_stayingAwake)
        qDebug() << "Error creating the power assertion:" << ret;
#endif

#ifdef Q_OS_LINUX
    /* logind holds the block for as long as the returned descriptor is open */
    QDBusInterface logind("org.freedesktop.login1", "/org/freedesktop/login1",
                          "org.freedesktop.login1.Manager", QDBusConnection::systemBus());
    QDBusReply<QDBusUnixFileDescriptor> reply = logind.call("Inhibit", "sleep:idle",
                                                            "Gemstone Imager", reason, "block");
    _inhibitFd = reply.isValid() ? ::dup(reply.value().fileDescriptor()) : -1;
    _stayingAwake = (_inhibitFd >= 0);
    if (!_stayingAwake)
        qDebug() << "Error inhibiting sleep through logind:" << reply.error().message();
#endif
}

void PowerSaveBlocker::removeBlock()
{
    if (!_stayingAwake)
        return;

#ifdef Q_OS_WIN
    PowerClearRequest(_powerRequest, PowerRequestDisplayRequired);
    CloseHandle(_powerRequest);
#endif
#ifdef Q_OS_DARWIN
    IOPMAssertionRelease(_assertion);
#endif
#ifdef Q_OS_LINUX
    ::close(_inhibitFd);
#endif
    /* Otherwise every write after the first ran without a block */
    _stayingAwake = false;
}
