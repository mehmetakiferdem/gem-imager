#include "priviligedprocess.h"
#include <qlocalsocket.h>
#include <filesystem>

PriviligedProcess::PriviligedProcess(QObject* parent)
    : _proc{parent}
{
    QObject::connect(&_server, &QLocalServer::newConnection, [this]()
    {
        onNewConnection();
    });
}

PriviligedProcess::~PriviligedProcess()
{
    _proc.kill();
    if(_peer != nullptr)
    {
        _peer->close();
        delete _peer;
    }
    _server.close();
}

void PriviligedProcess::setArguments(QStringList &args)
{
    _args = args;

    /*
     * Send the helper's output to our own stdout/stderr rather than to a file.
     * This used to log to $TMPDIR/<appname>.txt, which is a predictable path in
     * a directory other users can write to - on a shared machine someone can
     * pre-create or symlink it and catch, or redirect, the output of a process
     * we are about to run as root. Forwarding keeps the output visible when the
     * application is started from a terminal, without putting it anywhere
     * another user can reach.
     */
    _proc.setProcessChannelMode(QProcess::ForwardedChannels);
}

QProcess *PriviligedProcess::getQProcess()
{
    return &_proc;
}

void PriviligedProcess::start()
{
#if defined(Q_OS_UNIX)
    _proc.start("pkexec", _args);
#elif defined(Q_OS_WIN)
    _proc.start("powershell.exe", QStringList() << "Start" << _args);
#else
#error "Not implemented yet!"
#endif
}

bool PriviligedProcess::waitForFinished(int msecs)
{
    return _proc.waitForFinished(msecs);
}

QProcess::ExitStatus PriviligedProcess::exitStatus()
{
    return _proc.exitStatus();
}

void PriviligedProcess::kill()
{
    _proc.kill();
}

bool PriviligedProcess::startCommunicationChannel(QByteArray channelName)
{
    _channelName = channelName;
    _server.removeServer(channelName);
    if(false == _server.listen(channelName))
    {
        qDebug() << _server.errorString();
        return false;
    }
    return true;
}

bool PriviligedProcess::waitForCommunicationChannelReady()
{
    if(false == _server.hasPendingConnections())
    {
        if(false == _server.waitForNewConnection(10000))
        {
            return false;
        }
    }

    if(_peer != nullptr)
    {
        _peer->close();
        delete _peer;
    }

    _peer = _server.nextPendingConnection();
    return true;
}

bool PriviligedProcess::sendMessage(QByteArray msg, int msecs)
{
    if(_peer == nullptr)
    {
        qDebug() << "no peer found: ";
        return false;
    }

    msg.append('\n');

    if(msg.size() != _peer->write(msg) && _peer->waitForBytesWritten(msecs))
    {
        qDebug() << "send failed: " << _peer->errorString();
        return false;
    }

    QByteArray recvBuf = recvMessage();

    if(recvBuf.size() != 0 && recvBuf == "ok")
    {
        return true;
    }

    return false;
}

QByteArray PriviligedProcess::getValue(QByteArray key, int msecs)
{
    if(false == sendMessage(key)) return "";
    return recvMessage(msecs);
}

bool PriviligedProcess::checkValue(QByteArray key, QByteArray expected, int msecs)
{
    return getValue(key,msecs) == expected;
}

QByteArray PriviligedProcess::recvMessage(int msec)
{
    if(_peer == nullptr)
    {
        return QByteArray{0, Qt::Initialization::Uninitialized};
    }

    if(false == _peer->bytesAvailable() && false == _peer->waitForReadyRead(msec))
    {
        return QByteArray{0, Qt::Initialization::Uninitialized};
    }

    return _peer->readLine().trimmed();
}

void PriviligedProcess::onNewConnection()
{

}
