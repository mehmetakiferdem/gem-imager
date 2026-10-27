/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2024 Raspberry Pi Ltd
 */

#include "dfuthread.h"
#include "dfuwrapper.h"
#include "dfudriver.h"
#include "config.h"
#include "downloadextractthread.h"
#include <QFile>
#include <QDebug>
#include <QThread>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

DfuThread::DfuThread(const QByteArray &url, const QByteArray &localfilename,
                     const QByteArray &expectedHash, const QByteArray &tiboot3Hash,
                     const QByteArray &tisplHash, const QByteArray &ubootHash, QObject *parent)
    : DownloadExtractThread(url, localfilename, expectedHash, parent)
    , _tempImageFile(nullptr)
    , _expectedTiboot3Hash(tiboot3Hash)
    , _expectedTisplHash(tisplHash)
    , _expectedUbootHash(ubootHash)
{
    _suppressSuccessSignal = true;
    _ejectEnabled = false;
    _activeDfu = nullptr;
}

DfuThread::~DfuThread()
{
    for (int i = 0; i < 3; i++)
        if (!_bootloaderFiles[i].isEmpty())
            QFile::remove(_bootloaderFiles[i]);

    if (_tempImageFile) {
        _tempImageFile->remove();
        delete _tempImageFile;
    }
}

bool DfuThread::isImage()
{
    return true;
}

void DfuThread::cancelDownload()
{
    DownloadExtractThread::cancelDownload();
    if (_activeDfu)
        _activeDfu->cancel();
}

void DfuThread::setTempDirectory(const QString &dir)
{
    _tempDir = dir;
}

bool DfuThread::_openAndPrepareDevice()
{
    QString tempDir = _tempDir.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::CacheLocation) : _tempDir;
    QDir().mkpath(tempDir);
    _tempImageFile = new QTemporaryFile(tempDir + QDir::separator() + "dfu_image_XXXXXX");
    _tempImageFile->setAutoRemove(false);

    if (!_tempImageFile->open()) {
        emit error(tr("Failed to create temporary file for DFU image"));
        return false;
    }

    _tempImagePath = _tempImageFile->fileName();
    _tempImageFile->close();

    _filename = _tempImagePath.toLatin1();
    _file.setFileName(_tempImagePath);
    if (!_file.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        emit error(tr("Failed to open temporary file for DFU image"));
        return false;
    }

    return true;
}

void DfuThread::run()
{
    emit preparationStatusUpdate(tr("Initializing DFU..."));

    if (_url.isEmpty() || _url == "dfu") {
        emit error(tr("DFU mode requires an image URL"));
        return;
    }

    if (QUrl(QString::fromUtf8(_url)).isLocalFile())
        emit dfuProgress(5, tr("Reading image from cache/local file (no download needed)..."));
    else
        emit dfuProgress(5, tr("Downloading image..."));
    DownloadExtractThread::run();
    waitForExtractThread();
    if (!_successful) return;
    if (_cancelled) { emit error(tr("Cancelled")); return; }

    if (!_geminit.isEmpty() || !_config.isEmpty() || !_cmdline.isEmpty() || !_firstrun.isEmpty() || !_cloudinit.isEmpty()) {
        emit dfuProgress(35, tr("Customizing image..."));
        if (_file.isOpen()) _file.close();
        _file.setFileName(_tempImagePath);
        if (!_file.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
            emit error(tr("Failed to reopen image for customization: %1").arg(_file.errorString()));
            return;
        }
        if (!_customizeImage()) return;
        _file.close();
    }
    if (_cancelled) { emit error(tr("Cancelled")); return; }

    emit dfuProgress(38, tr("Fetching bootloader files..."));
    if (!fetchBootloaderFiles()) return;
    if (_cancelled) { emit error(tr("Cancelled")); return; }

    /* A missing Windows driver makes every stage below fail after its own
       retries, so check for it once here rather than letting the user wait out
       three rounds of that and then read a libusb error. A device that is not
       attached yet is not an error: the retry loops wait for it on purpose. */
    const QString driverHint = DfuDriver::missingDriverHint(DfuWrapper::TI_VENDOR_ID,
                                                            DfuWrapper::TI_PRODUCT_ID);
    if (!driverHint.isEmpty()) {
        emit error(driverHint);
        return;
    }

    emit dfuProgress(45, tr("Sending bootloader files..."));
    if (!sendBootloaderFiles()) return;
    if (_cancelled) { emit error(tr("Cancelled")); return; }

    emit dfuProgress(77, tr("Waiting for device to enter DFU mode..."));
    QThread::sleep(3);
    if (_cancelled) { emit error(tr("Cancelled")); return; }

    emit dfuProgress(80, tr("Sending image to device (this may take several minutes)..."));
    if (!sendImageToRawemmc()) return;

    /* All data has been sent, but the device is still flushing buffered image
       data to eMMC and then writes the boot binaries. U-Boot's DFU gadget only
       drops off the USB bus once all of that is done, so wait for the device
       to disappear instead of declaring success while it is still writing.
       (If a second board in DFU mode is attached this waits for that one too;
       that is the safe direction to be wrong in.) */
    emit dfuProgress(95, tr("Device is writing to eMMC (do not power off)..."));
    QElapsedTimer flushTimer;
    flushTimer.start();
    const qint64 flushTimeoutMs = 15 * 60 * 1000;
    while (DfuWrapper::isDevicePresent(DfuWrapper::TI_VENDOR_ID, DfuWrapper::TI_PRODUCT_ID)) {
        if (_cancelled) { emit error(tr("Cancelled")); return; }
        if (flushTimer.elapsed() > flushTimeoutMs) {
            emit error(tr("The device is still writing to eMMC %1 minutes after the "
                          "transfer finished.<br>Do NOT power off the board yet: check "
                          "the serial console and wait until write activity stops.")
                       .arg(flushTimeoutMs / 60000));
            return;
        }
        QThread::msleep(500);
    }
    /* The boot binaries are written right after the DFU gadget shuts down;
       give that step a moment to finish before declaring the board safe to
       power off. */
    QThread::sleep(5);

    emit dfuProgress(100, tr("Image written to eMMC successfully! Power off the board "
                             "and switch the boot mode to eMMC."));
    QThread::msleep(1000);
    emit success();
}

bool DfuThread::runDfu(const QString &altSetting, const QString &filePath, bool resetAfter)
{
    // Bootloader stages are cheap to redo; the multi-minute image stream gets one retry
    const int maxAttempts = resetAfter ? 3 : 2;
    QString lastError;

    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        if (_cancelled)
            return false;

        DfuWrapper *dfu = new DfuWrapper(nullptr);
        _activeDfu = dfu;

        if (_cancelled) dfu->cancel();

        if (!resetAfter)
            connect(dfu, &DfuWrapper::streamProgress, this, &DfuThread::onStreamProgress);

        bool ok = dfu->initialize()
               && dfu->findDevice(DfuWrapper::TI_VENDOR_ID, DfuWrapper::TI_PRODUCT_ID, altSetting)
               && (resetAfter ? dfu->downloadFile(filePath, true)
                              : dfu->downloadFileStreaming(filePath));

        bool cancelled = dfu->isCancelled();
        lastError = dfu->lastError();

        _activeDfu = nullptr;
        dfu->cleanup();
        delete dfu;

        if (ok)
            return true;
        if (cancelled || _cancelled)
            return false;

        if (attempt < maxAttempts) {
            qDebug() << "DFU attempt" << attempt << "failed for alt" << altSetting << ":" << lastError << "- retrying";
            emit dfuProgress(resetAfter ? 45 : 80,
                             tr("DFU transfer failed (%1), retrying...").arg(lastError));
            QThread::sleep(2);
        }
    }

    /* The driver can be unbound mid-run (the board re-enumerates between
       stages), and then power cycling is the wrong advice. */
    const QString driverHint = DfuDriver::missingDriverHint(DfuWrapper::TI_VENDOR_ID,
                                                            DfuWrapper::TI_PRODUCT_ID);
    if (!driverHint.isEmpty())
        emit error(tr("DFU transfer failed while sending %1: %2<br><br>%3")
                   .arg(altSetting, lastError, driverHint));
    else
        emit error(tr("DFU transfer failed while sending %1: %2<br><br>"
                      "Power off the board, set the boot switches to DFU mode again, "
                      "restore power and retry.").arg(altSetting, lastError));
    return false;
}

void DfuThread::onStreamProgress(qint64 bytesSent, qint64 totalBytes)
{
    if (totalBytes <= 0) return;
    int pct = 80 + (int)(15.0 * bytesSent / totalBytes);
    emit dfuProgress(pct, tr("Sending image: %1 / %2 MB (%3%)")
                     .arg(bytesSent / (1024 * 1024))
                     .arg(totalBytes / (1024 * 1024))
                     .arg((int)(100.0 * bytesSent / totalBytes)));
}

bool DfuThread::sendBootloaderFiles()
{
    const char *altSettings[] = {
        DfuWrapper::ALT_BOOTLOADER,
        DfuWrapper::ALT_TISPL,
        DfuWrapper::ALT_UBOOT,
    };

    for (int i = 0; i < 3; i++) {
        emit dfuProgress(45 + i * 10, tr("Sending %1...").arg(altSettings[i]));
        if (!runDfu(altSettings[i], _bootloaderFiles[i], true))
            return false;
        emit dfuProgress(55 + i * 10, tr("%1 sent").arg(altSettings[i]));

        if (i < 2) {
            emit dfuProgress(55 + i * 10, tr("Waiting for device to reconnect..."));
            QThread::sleep(2);
        }
    }

    return true;
}

bool DfuThread::fetchBootloaderFiles()
{
    QString listUrlStr = QString(BOOTIMG_URL).arg("t3-gem-o1").arg("list.json");
    
    emit dfuProgress(40, tr("Fetching bootloader list..."));

    QNetworkAccessManager manager;
    QNetworkRequest request((QUrl(listUrlStr)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray jsonData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonObject root = doc.object();
        QJsonArray filesArr = root["files"].toArray();
        
        for (int j = 0; j < filesArr.size(); ++j) {
            QJsonObject fileObj = filesArr[j].toObject();
            QString name = fileObj["name"].toString();
            QByteArray hash = fileObj["sha256"].toString().toUtf8();
            
            if (name == "tiboot3.bin") _expectedTiboot3Hash = hash;
            else if (name == "tispl.bin") _expectedTisplHash = hash;
            else if (name == "u-boot.img") _expectedUbootHash = hash;
        }
        qDebug() << "Updated bootloader hashes from list.json successfully.";
    } else {
        qDebug() << "Failed to fetch list.json from" << listUrlStr << ". Error:" << reply->errorString();
        if (_expectedTiboot3Hash.isEmpty() || _expectedTisplHash.isEmpty() || _expectedUbootHash.isEmpty()) {
            emit error(tr("Failed to fetch bootloader list and no fallback hashes available. "
                         "Check your network connection and try again."));
            reply->deleteLater();
            return false;
        }
    }
    reply->deleteLater();

    QStringList fileNames = {"tiboot3.bin", "tispl.bin", "u-boot.img"};
    QList<QByteArray> expectedHashes = {_expectedTiboot3Hash, _expectedTisplHash, _expectedUbootHash};

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QDir::separator() + "bootloaders" + QDir::separator() + "t3-gem-o1";
    QDir().mkpath(cacheDir);

    for (int i = 0; i < 3; i++) {
        QString fileName = fileNames[i];
        QByteArray expectedHash = expectedHashes[i];
        // Keep temp files out of /tmp: it is RAM-backed (tmpfs) on some distros
        QTemporaryFile tmp(cacheDir + QDir::separator() + "dfu_XXXXXX");
        tmp.setAutoRemove(false);
        if (!tmp.open()) {
            emit error(tr("Failed to create temp file for %1").arg(fileName));
            return false;
        }
        tmp.close();

        QString localCachePath = cacheDir + QDir::separator() + fileName;
        QFileInfo fi(localCachePath);

        QString urlstr;
        bool useCache = false;

        if (fi.exists() && fi.size() > 0) {
            if (!expectedHash.isEmpty()) {
                QFile f(localCachePath);
                if (f.open(QIODevice::ReadOnly)) {
                    QCryptographicHash hash(QCryptographicHash::Sha256);
                    if (hash.addData(&f)) {
                        if (hash.result().toHex() == expectedHash) {
                            useCache = true;
                        } else {
                            qDebug() << "Cache hash mismatch for" << fileName << "expected:" << expectedHash << "got:" << hash.result().toHex();
                        }
                    }
                    f.close();
                }
            } else {
                useCache = true;
            }
        }

        if (useCache) {
            urlstr = QUrl::fromLocalFile(localCachePath).toString(QUrl::FullyEncoded);
        } else {
            urlstr = QString(BOOTIMG_URL).arg("t3-gem-o1").arg(fileName);
            if (fi.exists()) {
                QFile::remove(localCachePath);
            }
        }

        DownloadThread dt(urlstr.toUtf8(), tmp.fileName().toUtf8(), expectedHash, true);
        if (urlstr.startsWith("http")) {
            dt.setCacheFile(localCachePath);
        }

        dt.start();
        dt.wait();

        if (!dt.successfull()) {
            emit error(tr("Failed to download bootloader file: %1").arg(fileName));
            QFile::remove(localCachePath); // clean up failed cache
            return false;
        }

        _bootloaderFiles[i] = tmp.fileName();
        qDebug() << "Ready" << fileName << "from" << urlstr;
    }

    return true;
}

bool DfuThread::sendImageToRawemmc()
{
    if (!QFile::exists(_tempImagePath)) {
        emit error(tr("Image not found: %1").arg(_tempImagePath));
        return false;
    }
    return runDfu(DfuWrapper::ALT_RAWEMMC, _tempImagePath, false);
}
