#ifndef CRASHLOG_H
#define CRASHLOG_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include <QString>

/*
 * The GUI build has no console on Windows, so a crash used to close the
 * window without leaving anything behind to tell what happened. This writes
 * every Qt log message to a file in the user's application data folder and
 * records crashes there as well: the signal or exception, a stack trace with
 * module offsets and, on Windows, a minidump next to the log.
 *
 * Lines go to the file unbuffered, so whatever was logged before a crash is
 * on disk even when the process dies without running any handler (a heap
 * corruption fast-fail, a kill from the task manager). Each session ends with
 * a clean exit line; the next start notes when the previous session lacks it.
 */
namespace CrashLog
{
    /* Opens a new log file and installs the message and crash handlers.
       Call once, after the organization and application names are set and
       before the application object is created. */
    void install(int argc, char *argv[]);

    /* Echo log messages to stderr as well (Windows --debug console). */
    void setConsoleEcho(bool enabled);

    /* Records that the session ended through a normal return from main(). */
    void markCleanExit(int exitCode);

    QString logDirectory();
    QString logFilePath();
}

#endif // CRASHLOG_H
