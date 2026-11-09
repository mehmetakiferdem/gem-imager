#ifndef MACFILE_H
#define MACFILE_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020 Raspberry Pi Ltd
 */

#include <QFile>

class MacFile : public QFile
{
    Q_OBJECT
public:
    enum authOpenResult {authOpenCancelled, authOpenSuccess, authOpenError };

    MacFile(QObject *parent = nullptr);
    virtual bool isSequential() const;
    authOpenResult authOpen(const QByteArray &filename);

    /* Why the last authOpen() returned authOpenError. Worth showing the user:
       "could not execute /usr/libexec/authopen" and "authopen exited with code
       1" are different problems with different fixes, and the difference used
       to be visible only in a terminal. */
    QString lastAuthOpenError() const { return _lastError; }

private:
    QString _lastError;
};

#endif // MACFILE_H
