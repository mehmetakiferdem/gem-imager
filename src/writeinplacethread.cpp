#include "writeinplacethread.h"
#include "archive.h"
#include "config.h"

WriteInPlaceThread::WriteInPlaceThread(
    const QByteArray &url,
    const QByteArray &localfilename,
    const QByteArray &expectedHash,
    const QByteArray &boardName,
    QObject *parent
)
    : DownloadExtractThread{url, localfilename, expectedHash, parent}, _boardName(boardName)
{
}

void WriteInPlaceThread::run()
{
    QScopedPointer<PriviligedProcess> bootpProc{};
    auto cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    auto startSimpBootp = [this, &bootpProc, &cacheDir]()
    {
        bootpProc.reset(new PriviligedProcess);
        if(false == bootpProc->startCommunicationChannel("SimpbootpCommChannel"))
        {
            emit error(tr("Error comm start with simpbootp server"));
            return;
        }

#if defined(Q_OS_UNIX)
        QString simpbootpBinaryPath;
        char* appimagedir = getenv("APPIMAGE");
        if(appimagedir)
        {
            // Use the cache dir, not TempLocation: /tmp is RAM-backed (tmpfs) on some distros
            QString bupPath = cacheDir + QDir::separator() + "GemImagerRelocated";
            // Beacause how appimage works we have to copy APPDIR to different directory for root user access
            QProcess::execute("cp", QStringList() << "-r" << getenv("APPDIR") << bupPath);
            simpbootpBinaryPath = bupPath + "/usr/bin/simpbootp";
            qDebug() << "binarypath: " << simpbootpBinaryPath;
        }
        else if(QFile::exists("./simpbootp"))
        {
            simpbootpBinaryPath = "./simpbootp";
        }
        else
        {
            qDebug() << "simpbootp bulunamadi";
            emit error("Simpbootp binary not found. Probably package corrupted!");
            return;
        }

        bootpProc->setArguments(
            QStringList() << simpbootpBinaryPath
            << "--interface" << _selEthPort
            << "--single-run" << "tiboot3.bin"
            << "--target-directory" << cacheDir
        );
#elif defined(Q_OS_WIN)
    QString simpbootCommand = QString("powershell.exe -WindowStyle Hidden -ArgumentList \"-ExecutionPolicy Bypass \
                            -Command `\"./simpbootp.exe --interface Ethernet  --target-directory %1 --single-run tiboot3.bin`\"\" -Verb RunAs").arg(cacheDir);

    bootpProc->setArguments(QStringList() << simpbootCommand);
#endif
        bootpProc->start();
    };

    if(_boardName.isEmpty())
    {
        emit error(tr("Uniflash not supported for this board. This error indicates corrupted json configuration!"));
        return;
    }

    // Downlaod boot files
    emit preparationStatusUpdate("Downloading boot files");

    QString fileUrl{BOOTIMG_URL};
    QVector<DownloadThread*> dlThreads{};
    QByteArray tiboot3Path = (cacheDir + QDir::separator() + "tiboot3.bin").toUtf8();
    QByteArray emmcBootPath = (cacheDir + QDir::separator() + "texas_am67_sbl_emmcboot.release.hs_fs.tiimage").toUtf8();
    QByteArray linuxAppimagePath = (cacheDir + QDir::separator() + "linux.appimage.hs_fs").toUtf8();
    QByteArray ubootImgPath = (cacheDir + QDir::separator() + "u-boot.img").toUtf8();
    QByteArray imageFilePath = (cacheDir + QDir::separator() + _filename).toUtf8();

    dlThreads.reserve(3);
    dlThreads.append(new DownloadThread(fileUrl.arg(_boardName, "texas_am67_sbl_gemboot.release.hs_fs.tiimage").toUtf8(), tiboot3Path , 0, true));
    dlThreads.append(new DownloadThread(fileUrl.arg(_boardName, "texas_am67_sbl_emmcboot.release.hs_fs.tiimage").toUtf8(), emmcBootPath, 0, true));
    dlThreads.append(new DownloadThread(fileUrl.arg(_boardName, "linux.appimage.hs_fs").toUtf8(), linuxAppimagePath, 0, true));
    dlThreads.append(new DownloadThread(fileUrl.arg(_boardName, "u-boot.img").toUtf8(), ubootImgPath, 0, true));

    QObject::connect(dlThreads.last(), &DownloadThread::preparationStatusUpdate, this, &WriteInPlaceThread::preparationStatusUpdate);
    QObject::connect(dlThreads.last(), &DownloadThread::updateNumProgress, this, &WriteInPlaceThread::updateNumProgress);

    for(auto& thread: dlThreads)
    {
        thread->start();
    }

    auto deleteBootFiles = QScopeGuard{[tiboot3Path, emmcBootPath, linuxAppimagePath, ubootImgPath]()
        {
            QFile::remove(tiboot3Path);
            QFile::remove(emmcBootPath);
            QFile::remove(linuxAppimagePath);
            QFile::remove(ubootImgPath);
        }
    };

    for(auto& thread: dlThreads)
    {
        if(false == thread->wait(10000))
        {
            emit error("Timeout while downloading boot files!");
            for(auto& killthread: dlThreads)
            {
                killthread->terminate();
                killthread->deleteLater();
            }
            return;
        }
    }

    dlThreads.last()->disconnect();

    for(auto& thread: dlThreads)
    {
        thread->deleteLater();
    }

    startSimpBootp();

    if(!QFile::exists(tiboot3Path) || !QFile::exists(linuxAppimagePath) || !QFile::exists(ubootImgPath))
    {
        emit error("Failed downloading boot files!");
        return;
    }

    DownloadExtractThread* th{ new DownloadExtractThread(_url, imageFilePath, _expectedHash) };
    bool isDownExtrSuccess{true};

    auto cleanup = QScopeGuard{[th, &bootpProc, imageFilePath]()
    {
        if(false == bootpProc->sendMessage("quit")) qDebug() << "send quit failed!";
        if(false == bootpProc->waitForFinished(3000))
        {
            bootpProc->kill();
            bootpProc->waitForFinished(1000);
        }

        if (th)
        {
            th->terminate();
            th->deleteLater();
        }
        QFile::remove(imageFilePath);
    }};

    QEventLoop loop;
    emit preparationStatusUpdate("Downloading image");
    QObject::connect(th, &DownloadExtractThread::updateNumProgress, this, [this](QVariant pos)
    {
        qDebug() << "updateNumProgress()" << pos;
        emit this->updateNumProgress(pos);
    });

    QObject::connect(th, &DownloadExtractThread::preparationStatusUpdate, this, [this](QString msg)
    {
        qDebug() << "preparationStatusUpdate()" << msg;
        emit this->preparationStatusUpdate(msg);
    });

    QObject::connect(th, &DownloadExtractThread::success, &loop, &QEventLoop::quit);
    QObject::connect(th, &DownloadExtractThread::error, &loop, [&loop, &isDownExtrSuccess](QString err_msg)
    {
        qDebug() << "Download extract failed: " << err_msg;
        isDownExtrSuccess = false;
        loop.quit();
    });

    th->setVerifyEnabled(_verifyEnabled);
    th->setUserAgent(QString("Mozilla/5.0 gem-imager/%1").arg("1.0").toUtf8());
    th->setImageCustomization(_config, _cmdline, _firstrun, _cloudinit, _cloudinitNetwork, _geminit, _initFormat, _destination);

    th->start();
    loop.exec();

    if(!isDownExtrSuccess)
    {
        emit error(tr("Download extract failed!"));
        return;
    }

    emit preparationStatusUpdate("The board is awaiting to boot");

    if(false == bootpProc->waitForCommunicationChannelReady())
    {
        emit error(tr("Error connecting Simpbootp server"));
        return;
    }

    if(false == bootpProc->checkValue("isSblUartReady", "ok"))
    {
        if(false == bootpProc->sendMessage("notifyFileSend")) qDebug() << "notify set send failed!";
        auto resp = bootpProc->recvMessage(10000);

        if(resp != "tiboot3.bin")
        {
            qDebug() << "SBL Uart is not ready but no reason to exit here. filename: " << resp;
        }
    }

    Transfer* transferInstance{ new Transfer(_selSerPort, _serPortbaudRate, _filename) };
    if(false == transferInstance->setSerialPortAndConfigure(_selSerPort, UNIFLASH_BAUD_RATE))
    {
        emit error(tr("Error starting communication with board"));
        return;
    }

    sendFileViaXModem(transferInstance, linuxAppimagePath);
    if(false == waitForSendFileViaXModemCompleted(transferInstance))
    {
        emit error(tr("Error sending file with XMODEM: ") + tr(_lastErrorString.toUtf8()));
        return;
    }

    transferInstance->terminate();
    transferInstance->wait(1000);
    transferInstance->deleteLater();

    transferInstance =  new Transfer(_selSerPort, _serPortbaudRate, _filename);
    if(false == transferInstance->setSerialPortAndConfigure(_selSerPort, UNIFLASH_BAUD_RATE))
    {
        emit error(tr("Error starting communication with board"));
        return;
    }

    bootpProc->sendMessage("1000MB");

    sendFileViaXModem(transferInstance, ubootImgPath);
    if(false == waitForSendFileViaXModemCompleted(transferInstance))
    {
        emit error(tr("Error sending file with XMODEM: ") + tr(_lastErrorString.toUtf8()));
        return;
    }

    transferInstance->terminate();
    transferInstance->wait(1000);
    transferInstance->deleteLater();

    QTimer progressTimer;
    bool imageSendFailed{false};

    progressTimer.setInterval(1000);
    progressTimer.callOnTimeout([this, &progressTimer, &bootpProc, &loop, &imageSendFailed](){
        auto status_str = bootpProc->getValue("isFileTransferOk");
        if(status_str == "nok")
        {
            imageSendFailed = true;
            loop.quit();
        }

        auto progress_str = bootpProc->getValue("getFileSendProgress");
        float progress = progress_str.toFloat();
        updateNumProgress(progress);

        if(progress >= 1.0)
        {
            loop.quit();
        }

        if(_cancelled)
        {
            loop.quit();
        }
    });

    progressTimer.start();
    emit updateNumProgress(QVariant{0.0});
    loop.exec();

    if(imageSendFailed)
    {
        emit error(tr("TFTP server failed to send image file!"));
        return;
    }

    if(_cancelled)
    {
        emit error(tr("Process cancelled by user. Please power cycle the board before retrying!"));
        return;
    }

    emit success();
}

void WriteInPlaceThread::setSerPortbaudRate(uint32_t newSerPortbaudRate)
{
    _serPortbaudRate = newSerPortbaudRate;
}

void WriteInPlaceThread::setPortNames(QString selectedSerialPort, QString selectedEthernetPort)
{
    _selSerPort = selectedSerialPort;
    _selEthPort = selectedEthernetPort;
}

bool WriteInPlaceThread::waitForSendFileViaXModemCompleted(Transfer* transferInstance, uint32_t timeout)
{
    bool result{true};
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout);
    timer.callOnTimeout([&loop, &result]()
    {
        qDebug() << "waitForSendFileViaXModemCompleted timeout";
        result = false;
        loop.quit();
    });

    QObject::connect(transferInstance, &Transfer::transferCompleted, &loop, &QEventLoop::quit);
    QObject::connect(transferInstance, &Transfer::transferFailed, &loop, [&loop, &result](QString err)
    {
        qDebug() << err;
        result = false;
        loop.quit();
    });
    timer.start();
    loop.exec();
    timer.stop();

    return result;
}

void WriteInPlaceThread::sendFileViaXModem(Transfer* transferInstance, const QString& filePath){
    transferInstance->setPkcsPadding(true);
    transferInstance->setFilePath(filePath);
    _isSendFileViaXModemCompleted = false;

    //Connect to signals
    connect(transferInstance, SIGNAL(updateProgress(float)), this, SLOT(updateProgress(float)));
    connect(transferInstance, SIGNAL(transferCompleted(void)), this, SLOT(onTransferCompleted(void)));
    connect(transferInstance, SIGNAL(transferFailed(QString)), this, SLOT(onTransferFailed(QString)));

    //Launch
    transferInstance->launch();
}

void WriteInPlaceThread::updateProgress(float progress){
    progress = (progress > 1.0) ? 1.0 : progress;
    progress = (progress < 0.0) ? 0.0 : progress;
    emit updateNumProgress(progress);
}

void WriteInPlaceThread::onTransferCompleted(){
    _isSendFileViaXModemCompleted = true;
    _isSendFileViaXModemCompletedSuccessfull = true;
}

void WriteInPlaceThread::onTransferFailed(QString reason){
    _isSendFileViaXModemCompleted = true;
    _isSendFileViaXModemCompletedSuccessfull = false;
    _lastErrorString = reason;
}

WriteInPlaceThread::~WriteInPlaceThread()
{
    _cancelled = true;
    _cancelExtract();
}
