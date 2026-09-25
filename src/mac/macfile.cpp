/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020 Raspberry Pi Ltd
 */

#include "macfile.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <security/Authorization.h>
#include <QDebug>

MacFile::MacFile(QObject *parent)
    : QFile(parent)
{

}

/* Prevent that Qt thinks /dev/rdisk does not permit seeks because it does not report size */
bool MacFile::isSequential() const
{
    return false;
}

MacFile::authOpenResult MacFile::authOpen(const QByteArray &filename)
{
    int fd = -1;

    QByteArray right = "sys.openfile.readwrite."+filename;
    AuthorizationItem item = {right, 0, nullptr, 0};
    AuthorizationRights rights = {1, &item};
    AuthorizationFlags flags = kAuthorizationFlagInteractionAllowed |
            kAuthorizationFlagExtendRights |
            kAuthorizationFlagPreAuthorize;
    AuthorizationRef authRef;
    if (AuthorizationCreate(&rights, nullptr, flags, &authRef) != 0)
        return authOpenCancelled;

    AuthorizationExternalForm externalForm;
    if (AuthorizationMakeExternalForm(authRef, &externalForm) != 0)
    {
        _lastError = QStringLiteral("AuthorizationMakeExternalForm() failed");
        qDebug() << _lastError;
        AuthorizationFree(authRef, 0);
        return authOpenError;
    }

    const char *cmd = "/usr/libexec/authopen";
    QByteArray mode = QByteArray::number(O_RDWR);
    int pipe[2];
    int stdinpipe[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, pipe) == -1)
    {
        _lastError = QStringLiteral("socketpair() failed: %1").arg(strerror(errno));
        qDebug() << _lastError;
        AuthorizationFree(authRef, 0);
        return authOpenError;
    }
    if (::pipe(stdinpipe) == -1)
    {
        _lastError = QStringLiteral("pipe() failed: %1").arg(strerror(errno));
        qDebug() << _lastError;
        ::close(pipe[0]);
        ::close(pipe[1]);
        AuthorizationFree(authRef, 0);
        return authOpenError;
    }
    pid_t pid = ::fork();
    if (pid == -1)
    {
        _lastError = QStringLiteral("fork() failed: %1").arg(strerror(errno));
        qDebug() << _lastError;
        ::close(pipe[0]);
        ::close(pipe[1]);
        ::close(stdinpipe[0]);
        ::close(stdinpipe[1]);
        AuthorizationFree(authRef, 0);
        return authOpenError;
    }
    if (pid == 0)
    {
        // child
        ::close(pipe[0]);
        ::close(stdinpipe[1]);
        ::dup2(pipe[1], STDOUT_FILENO);
        ::dup2(stdinpipe[0], STDIN_FILENO);
        ::execl(cmd, cmd, "-stdoutpipe", "-extauth", "-o", mode.data(), filename.data(), NULL);
        ::exit(-1);
    }
    else
    {
        ::close(pipe[1]);
        ::close(stdinpipe[0]);
        ::write(stdinpipe[1], externalForm.bytes, sizeof(externalForm.bytes));
        ::close(stdinpipe[1]);

        const size_t bufSize = CMSG_SPACE(sizeof(int));
        char buf[bufSize];
        struct iovec io_vec[1];
        io_vec[0].iov_base = buf;
        io_vec[0].iov_len = bufSize;
        const size_t cmsgSize = CMSG_SPACE(sizeof(int));
        char cmsg[cmsgSize];

        struct msghdr msg = {0};
        msg.msg_iov = io_vec;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsg;
        msg.msg_controllen = cmsgSize;

        ssize_t size;
        do {
            size = recvmsg(pipe[0], &msg, 0);
        } while (size == -1 && errno == EINTR);

        qDebug() << "RECEIVED SIZE:" << size;

        if (size > 0) {
            struct cmsghdr *chdr = CMSG_FIRSTHDR(&msg);
            if (chdr && chdr->cmsg_type == SCM_RIGHTS) {
                qDebug() << "SCMRIGHTS";
                fd = *( (int*) (CMSG_DATA(chdr)) );
            }
            else
            {
                qDebug() << "NOT SCMRIGHTS";
            }
        }

        pid_t wpid;
        int status;

        do {
            wpid = ::waitpid(pid, &status, 0);
        } while (wpid == -1 && errno == EINTR);

        /* Done with the read end either way; it used to be left open, leaking
           one descriptor per call. */
        ::close(pipe[0]);

        if (wpid == -1)
        {
            _lastError = QStringLiteral("waitpid() failed executing authopen: %1").arg(strerror(errno));
            qDebug() << _lastError;
            if (fd != -1) ::close(fd);
            AuthorizationFree(authRef, 0);
            return authOpenError;
        }
        if (WEXITSTATUS(status))
        {
            /* 255 is our own exit(-1) from the child, i.e. authopen could not
               be executed at all - a different problem from authopen running
               and refusing. Carry the code so the caller can say which. */
            _lastError = (WEXITSTATUS(status) == 255)
                ? QStringLiteral("could not execute %1").arg(cmd)
                : QStringLiteral("authopen exited with code %1").arg(WEXITSTATUS(status));
            qDebug() << "authopen returned failure code" << WEXITSTATUS(status);
            if (fd != -1) ::close(fd);
            AuthorizationFree(authRef, 0);
            return authOpenError;
        }
        if (fd == -1)
        {
            _lastError = QStringLiteral("authopen succeeded but passed no file descriptor");
            qDebug() << _lastError;
            AuthorizationFree(authRef, 0);
            return authOpenError;
        }

        qDebug() << "fd received:" << fd;
    }
    AuthorizationFree(authRef, 0);

    return open(fd, QIODevice::ReadWrite | QIODevice::ExistingOnly | QIODevice::Unbuffered, QFileDevice::AutoCloseHandle) ? authOpenSuccess : authOpenError;
}
