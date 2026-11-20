#ifndef POWERSAVEBLOCKER_H
#define POWERSAVEBLOCKER_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020 Raspberry Pi Ltd
 */

#include <QObject>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_DARWIN
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class PowerSaveBlocker : public QObject
{
    Q_OBJECT
public:
    explicit PowerSaveBlocker(QObject *parent = nullptr);
    virtual ~PowerSaveBlocker();
    void applyBlock(const QString &reason);
    void removeBlock();

signals:

protected:
    bool _stayingAwake;

#ifdef Q_OS_WIN
    HANDLE _powerRequest;
#endif
#ifdef Q_OS_DARWIN
    IOPMAssertionID _assertion;
#endif
#ifdef Q_OS_LINUX
    int _inhibitFd;
#endif
};

#endif // POWERSAVEBLOCKER_H
