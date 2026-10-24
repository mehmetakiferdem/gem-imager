/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include "crashlog.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QThread>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <mutex>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbghelp.h>
#ifndef UNW_FLAG_NHANDLER
#define UNW_FLAG_NHANDLER 0
#endif
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#if __has_include(<execinfo.h>)
#include <execinfo.h>
#define HAVE_EXECINFO
#endif
#endif

namespace {

/* Logs (and dumps) of this many sessions, the current one included, are kept */
const int keptSessions = 10;
const char sessionEndMarker[] = "=== Session ended";

QString logDir;
QString logPath;
std::mutex writeMutex;
bool consoleEcho = false;
bool cleanExit = false;
std::atomic<bool> crashing(false);
QtMessageHandler previousHandler = nullptr;

#ifdef Q_OS_WIN
typedef BOOL (WINAPI *MiniDumpWriteDumpFn)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
                                           PMINIDUMP_EXCEPTION_INFORMATION,
                                           PMINIDUMP_USER_STREAM_INFORMATION,
                                           PMINIDUMP_CALLBACK_INFORMATION);
HANDLE logHandle = INVALID_HANDLE_VALUE;
MiniDumpWriteDumpFn miniDumpWriteDump = nullptr;
/* Prepared up front: the crash handlers must not allocate */
wchar_t dumpPathW[MAX_PATH] = {0};
char dumpPathUtf8[MAX_PATH * 3] = {0};
#else
int logFd = -1;
char altStack[64 * 1024];
#endif

/*
 * Fixed-size line builder for the crash handlers, which cannot rely on the
 * heap or on printf being usable once the process state is corrupt.
 */
class LineBuffer
{
public:
    LineBuffer &str(const char *s)
    {
        while (s && *s && _len < sizeof(_buf))
            _buf[_len++] = *s++;
        return *this;
    }

    LineBuffer &hex(quint64 v)
    {
        char tmp[16];
        int n = 0;
        do {
            tmp[n++] = "0123456789abcdef"[v & 0xf];
            v >>= 4;
        } while (v);
        str("0x");
        while (n && _len < sizeof(_buf))
            _buf[_len++] = tmp[--n];
        return *this;
    }

    LineBuffer &dec(qint64 v)
    {
        char tmp[20];
        int n = 0;
        quint64 u = v < 0 ? quint64(-v) : quint64(v);
        do {
            tmp[n++] = char('0' + u % 10);
            u /= 10;
        } while (u);
        if (v < 0)
            str("-");
        while (n && _len < sizeof(_buf))
            _buf[_len++] = tmp[--n];
        return *this;
    }

    const char *data() const { return _buf; }
    size_t size() const { return _len; }

private:
    char _buf[1024];
    size_t _len = 0;
};

/* Unbuffered and lock-free, so it is usable from the crash handlers too */
void writeRaw(const char *data, size_t len)
{
#ifdef Q_OS_WIN
    if (logHandle == INVALID_HANDLE_VALUE)
        return;
    DWORD written;
    WriteFile(logHandle, data, DWORD(len), &written, nullptr);
#else
    if (logFd < 0)
        return;
    while (len)
    {
        ssize_t n = ::write(logFd, data, len);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return;
        data += n;
        len -= size_t(n);
    }
#endif
}

void writeLine(const LineBuffer &b)
{
    writeRaw(b.data(), b.size());
    writeRaw("\n", 1);
}

void writeLocked(const QByteArray &data)
{
    std::lock_guard<std::mutex> lock(writeMutex);
    writeRaw(data.constData(), size_t(data.size()));
}

const char *levelName(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg:
        return "D";
    case QtInfoMsg:
        return "I";
    case QtWarningMsg:
        return "W";
    case QtCriticalMsg:
        return "C";
    case QtFatalMsg:
        return "F";
    }
    return "?";
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray line = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")).toUtf8();
    line += ' ';
    line += levelName(type);
    line += " [";
    line += QByteArray::number(quint64(quintptr(QThread::currentThreadId())));
    line += "] ";
    if (context.category && strcmp(context.category, "default") != 0)
    {
        line += context.category;
        line += ": ";
    }
    line += msg.toUtf8();
    line += '\n';
    writeLocked(line);

    if (consoleEcho)
    {
        const QByteArray text = msg.toUtf8() + '\n';
        std::fwrite(text.constData(), 1, size_t(text.size()), stderr);
        std::fflush(stderr);
    }
    else if (previousHandler)
    {
        previousHandler(type, context, msg);
    }
}

void atExit()
{
    if (!cleanExit)
        writeLocked(QByteArray(sessionEndMarker) + " before reaching the normal shutdown path\n");
}

void terminateHandler()
{
    LineBuffer b;
    b.str("\n=== FATAL std::terminate called");
    if (std::exception_ptr e = std::current_exception())
    {
        try {
            std::rethrow_exception(e);
        } catch (const std::exception &ex) {
            b.str(" after an uncaught exception: ").str(ex.what());
        } catch (...) {
            b.str(" after an uncaught non-standard exception");
        }
    }
    writeLine(b);
    /* abort() lands in the SIGABRT handler, which logs the stack */
    std::abort();
}

#ifdef Q_OS_WIN

const char *exceptionName(DWORD code)
{
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        return "access violation";
    case EXCEPTION_STACK_OVERFLOW:
        return "stack overflow";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        return "illegal instruction";
    case EXCEPTION_PRIV_INSTRUCTION:
        return "privileged instruction";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return "integer divide by zero";
    case EXCEPTION_IN_PAGE_ERROR:
        return "in-page error";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        return "datatype misalignment";
    case 0xC0000409:
        return "stack buffer overrun / fail fast";
    case 0xC0000374:
        return "heap corruption";
    case 0x20474343:
        return "uncaught C++ exception (GCC)";
    case 0xE06D7363:
        return "uncaught C++ exception (MSVC)";
    }
    return "unknown exception";
}

void appendAddress(LineBuffer &b, quint64 addr)
{
    b.hex(addr);
    HMODULE module = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(addr), &module))
    {
        char name[MAX_PATH];
        DWORD n = GetModuleFileNameA(module, name, sizeof(name));
        if (n && n < sizeof(name))
        {
            const char *base = name;
            for (const char *p = name; *p; p++)
            {
                if (*p == '\\' || *p == '/')
                    base = p + 1;
            }
            b.str("  ").str(base).str("+").hex(addr - quint64(quintptr(module)));
        }
    }
}

/* Walks the stack from the given register state with the unwind tables
   every x64 module carries, so frames below an exception are included */
void writeStack(CONTEXT *ctx)
{
    writeRaw("Stack:\n", 7);
#if defined(_M_X64) || defined(__x86_64__)
    for (int frame = 0; frame < 64 && ctx->Rip; frame++)
    {
        LineBuffer b;
        b.str("  #").dec(frame).str(" ");
        appendAddress(b, ctx->Rip);
        writeLine(b);

        DWORD64 imageBase = 0;
        PRUNTIME_FUNCTION function = RtlLookupFunctionEntry(ctx->Rip, &imageBase, nullptr);
        if (function)
        {
            PVOID handlerData;
            DWORD64 establisherFrame;
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, ctx->Rip, function, ctx,
                             &handlerData, &establisherFrame, nullptr);
        }
        else
        {
            /* Leaf function, or a call through a bad pointer: the return
               address is on top of the stack */
            ctx->Rip = *reinterpret_cast<DWORD64 *>(ctx->Rsp);
            ctx->Rsp += 8;
        }
    }
#else
    Q_UNUSED(ctx);
    void *frames[62];
    USHORT n = CaptureStackBackTrace(0, 62, frames, nullptr);
    for (USHORT i = 0; i < n; i++)
    {
        LineBuffer b;
        b.str("  #").dec(i).str(" ");
        appendAddress(b, quint64(quintptr(frames[i])));
        writeLine(b);
    }
#endif
}

void writeMiniDump(EXCEPTION_POINTERS *ep)
{
    if (!miniDumpWriteDump || !dumpPathW[0])
        return;

    HANDLE f = CreateFileW(dumpPathW, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f == INVALID_HANDLE_VALUE)
        return;

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;
    BOOL ok = miniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), f,
                                MINIDUMP_TYPE(MiniDumpWithThreadInfo | MiniDumpWithIndirectlyReferencedMemory),
                                ep ? &mei : nullptr, nullptr, nullptr);
    CloseHandle(f);

    LineBuffer b;
    b.str(ok ? "Minidump written to " : "Writing the minidump failed: ").str(dumpPathUtf8);
    writeLine(b);
}

LONG WINAPI unhandledExceptionFilter(EXCEPTION_POINTERS *ep)
{
    if (crashing.exchange(true))
        return EXCEPTION_CONTINUE_SEARCH;

    const EXCEPTION_RECORD *er = ep->ExceptionRecord;
    LineBuffer b;
    b.str("\n=== FATAL ").str(exceptionName(er->ExceptionCode)).str(" (").hex(er->ExceptionCode).str(") at ");
    appendAddress(b, quint64(quintptr(er->ExceptionAddress)));
    if ((er->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || er->ExceptionCode == EXCEPTION_IN_PAGE_ERROR)
        && er->NumberParameters >= 2)
    {
        const ULONG_PTR op = er->ExceptionInformation[0];
        b.str(op == 0 ? ", reading " : op == 1 ? ", writing " : ", executing ").hex(er->ExceptionInformation[1]);
    }
    b.str(", thread ").dec(GetCurrentThreadId());
    writeLine(b);

    /* The walk modifies the context; the minidump needs the original */
    CONTEXT ctx = *ep->ContextRecord;
    writeStack(&ctx);
    writeMiniDump(ep);

    /* Let Windows Error Reporting have it as well */
    return EXCEPTION_CONTINUE_SEARCH;
}

void abortHandler(int)
{
    if (crashing.exchange(true))
        return;

    LineBuffer b;
    b.str("\n=== FATAL abort() called, thread ").dec(GetCurrentThreadId());
    writeLine(b);

    CONTEXT ctx;
    RtlCaptureContext(&ctx);
    writeStack(&ctx);
    writeMiniDump(nullptr);
}

void installCrashHandlers()
{
    /* Leaves the exception filter some stack to run on after a stack overflow */
    ULONG guarantee = 32 * 1024;
    SetThreadStackGuarantee(&guarantee);

    /* Loaded now rather than in the middle of a crash */
    if (HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll"))
        miniDumpWriteDump = reinterpret_cast<MiniDumpWriteDumpFn>(
            reinterpret_cast<void *>(GetProcAddress(dbghelp, "MiniDumpWriteDump")));

    SetUnhandledExceptionFilter(unhandledExceptionFilter);
    std::signal(SIGABRT, abortHandler);
}

#else // !Q_OS_WIN

const char *signalName(int sig)
{
    switch (sig)
    {
    case SIGSEGV:
        return "SIGSEGV";
    case SIGBUS:
        return "SIGBUS";
    case SIGILL:
        return "SIGILL";
    case SIGFPE:
        return "SIGFPE";
    case SIGABRT:
        return "SIGABRT";
    }
    return "signal";
}

void signalHandler(int sig, siginfo_t *info, void *)
{
    if (!crashing.exchange(true))
    {
        LineBuffer b;
        b.str("\n=== FATAL ").str(signalName(sig)).str(" (").dec(sig).str(")");
        if (info->si_code <= 0)
            b.str(" sent by process ").dec(info->si_pid);
        else if (sig != SIGABRT)
            b.str(" at address ").hex(quint64(quintptr(info->si_addr)));
        writeLine(b);
#ifdef HAVE_EXECINFO
        void *frames[64];
        int n = backtrace(frames, 64);
        writeRaw("Stack:\n", 7);
        backtrace_symbols_fd(frames, n, logFd);
#endif
    }

    /* SA_RESETHAND restored the default action: die the way we would have */
    raise(sig);
}

void installCrashHandlers()
{
#ifdef HAVE_EXECINFO
    /* The first call loads libgcc, which must not happen inside the handler */
    void *frame;
    backtrace(&frame, 1);
#endif

    stack_t ss = {};
    ss.ss_sp = altStack;
    ss.ss_size = sizeof(altStack);
    sigaltstack(&ss, nullptr);

    struct sigaction sa = {};
    sa.sa_sigaction = signalHandler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);
    for (int sig : {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT})
        sigaction(sig, &sa, nullptr);
}

#endif // Q_OS_WIN

bool openLogFile(const QString &path)
{
#ifdef Q_OS_WIN
    logHandle = CreateFileW(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(path).utf16()),
                            FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    return logHandle != INVALID_HANDLE_VALUE;
#else
    logFd = ::open(QFile::encodeName(path).constData(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND | O_CLOEXEC, 0644);
    return logFd >= 0;
#endif
}

/* Returns the previous session's log if it does not end with a session end
   line, i.e. that session crashed, was killed or is still running */
QString previousUncleanLog(const QFileInfoList &previousLogs)
{
    if (previousLogs.isEmpty())
        return QString();

    QFile f(previousLogs.first().absoluteFilePath());
    if (!f.open(QIODevice::ReadOnly))
        return QString();
    if (f.size() > 4096)
        f.seek(f.size() - 4096);
    if (f.readAll().contains(sessionEndMarker))
        return QString();
    return f.fileName();
}

void removeOldSessions(const QDir &dir, const QFileInfoList &previousLogs)
{
    for (int i = keptSessions - 1; i < previousLogs.size(); i++)
    {
        const QFileInfo &fi = previousLogs.at(i);
        QFile::remove(fi.absoluteFilePath());
        QFile::remove(dir.absoluteFilePath(fi.completeBaseName() + ".dmp"));
    }
}

} // namespace

void CrashLog::install(int argc, char *argv[])
{
    if (!logPath.isEmpty())
        return;

    logDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/logs";
    if (!QDir().mkpath(logDir))
        logDir = QDir::tempPath();
    QDir dir(logDir);

    const QFileInfoList previousLogs = dir.entryInfoList({"gem-imager-*.log"}, QDir::Files, QDir::Name | QDir::Reversed);
    const QString uncleanLog = previousUncleanLog(previousLogs);
    removeOldSessions(dir, previousLogs);

    const QDateTime now = QDateTime::currentDateTime();
    const QString baseName = QString("gem-imager-%1-%2").arg(now.toString("yyyyMMdd-HHmmss")).arg(QCoreApplication::applicationPid());
    logPath = dir.absoluteFilePath(baseName + ".log");
    if (!openLogFile(logPath))
    {
        logPath.clear();
        return;
    }

#ifdef Q_OS_WIN
    const QString dumpPath = QDir::toNativeSeparators(dir.absoluteFilePath(baseName + ".dmp"));
    if (dumpPath.size() < MAX_PATH)
    {
        dumpPath.toWCharArray(dumpPathW);
        dumpPathW[dumpPath.size()] = 0;
        qstrncpy(dumpPathUtf8, dumpPath.toUtf8().constData(), sizeof(dumpPathUtf8));
    }
#endif

    QByteArray header;
    header += "=== Gemstone Imager " IMAGER_VERSION_STR " session started " + now.toString(Qt::ISODateWithMs).toUtf8() + "\n";
    header += "OS: " + QSysInfo::prettyProductName().toUtf8() + " (" + QSysInfo::kernelType().toUtf8() + " "
              + QSysInfo::kernelVersion().toUtf8() + ", " + QSysInfo::currentCpuArchitecture().toUtf8() + ")\n";
    header += QByteArray("Qt: ") + qVersion() + " (built against " QT_VERSION_STR ")\n";
    header += "Command line:";
    for (int i = 0; i < argc; i++)
        header += " " + QString::fromLocal8Bit(argv[i]).toUtf8();
    header += "\n";
    header += "Log: " + QDir::toNativeSeparators(logPath).toUtf8() + "\n";
    if (!uncleanLog.isEmpty())
        header += "Previous session did not end normally (crashed, was killed or is still running): "
                  + QDir::toNativeSeparators(uncleanLog).toUtf8() + "\n";
    writeLocked(header);

    installCrashHandlers();
    std::set_terminate(terminateHandler);
    std::atexit(atExit);
    previousHandler = qInstallMessageHandler(messageHandler);
}

void CrashLog::setConsoleEcho(bool enabled)
{
    consoleEcho = enabled;
}

void CrashLog::markCleanExit(int exitCode)
{
    cleanExit = true;
    writeLocked(QByteArray(sessionEndMarker) + " normally (exit code " + QByteArray::number(exitCode) + ")\n");
}

QString CrashLog::logDirectory()
{
    return logDir;
}

QString CrashLog::logFilePath()
{
    return logPath;
}
