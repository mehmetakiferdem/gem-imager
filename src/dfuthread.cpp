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

void DfuThread::setStreamImageSize(qint64 imageSize)
{
    _streaming = imageSize > 0;
    _streamImageSize = imageSize;
}

/*
 * The extractor pads a final short block out to a multiple of the sector size,
 * so the number of bytes that reach _writeFile can exceed the declared image
 * length. Declare the padded length to the transfer, or it would report an
 * overrun on the last block of an unaligned image.
 */
static qint64 paddedTo512(qint64 n)
{
    return (n + 511) & ~511LL;
}

bool DfuThread::openStreamToRawemmc()
{
    DfuWrapper *dfu = new DfuWrapper(nullptr);
    _activeDfu = dfu;
    connect(dfu, &DfuWrapper::statusMessage, this, [this](const QString &m) {
        emit preparationStatusUpdate(m);
    });
    connect(dfu, &DfuWrapper::streamProgress, this, &DfuThread::onStreamProgress);

    if (!dfu->initialize()) {
        emit error(tr("Failed to initialise USB: %1").arg(dfu->lastError()));
        goto fail;
    }
    /* U-Boot needs a moment after the bootloader stage before it offers the
       rawemmc alt-setting, and the caller has already waited; still allow a few
       attempts rather than failing on the first miss. */
    for (int attempt = 1; attempt <= 15; attempt++) {
        if (_cancelled)
            goto fail;
        if (dfu->findDevice(DfuWrapper::TI_VENDOR_ID, DfuWrapper::TI_PRODUCT_ID,
                            DfuWrapper::ALT_RAWEMMC))
            break;
        if (attempt == 15) {
            emit error(tr("No DFU device found (VID:0x%1 PID:0x%2 alt:%3) after %4 retries")
                       .arg(DfuWrapper::TI_VENDOR_ID, 4, 16, QChar('0'))
                       .arg(DfuWrapper::TI_PRODUCT_ID, 4, 16, QChar('0'))
                       .arg(DfuWrapper::ALT_RAWEMMC).arg(attempt));
            goto fail;
        }
        QThread::msleep(1000);
    }

    if (!dfu->beginStream(paddedTo512(_streamImageSize))) {
        emit error(tr("Failed to start the DFU transfer: %1").arg(dfu->lastError()));
        goto fail;
    }
    return true;

fail:
    _activeDfu = nullptr;
    dfu->cleanup();
    delete dfu;
    return false;
}

size_t DfuThread::_writeFile(const char *buf, size_t len)
{
    if (!_streaming)
        return DownloadThread::_writeFile(buf, len);

    if (_cancelled || _streamFailed)
        return len;

    /* Kept in step with the base class so the image hash is still verified.
       Unlike the base class nothing is held back: the first block cannot be
       deferred to the end of a sequential transfer, so it goes out in order
       like every other block. */
    _writehash.addData(buf, len);

    if (!_activeDfu || !_activeDfu->streamChunk(buf, (qint64)len)) {
        _streamFailed = true;
        _streamError = _activeDfu ? _activeDfu->lastError() : tr("transfer was not open");
        /* Returning short makes the base class call _onWriteError(), which is
           overridden below to report _streamError. Reporting it from here as
           well would put two dialogs in front of the user. */
        return 0;
    }

    _bytesWritten += len;
    return len;
}

bool DfuThread::_openAndPrepareDevice()
{
    if (_streaming)
        return openStreamToRawemmc();

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

    /*
     * Streaming sends the image as it arrives, so the board has to be running
     * U-Boot and offering the rawemmc alt-setting *before* the download starts.
     * The bootloader stage therefore runs first here, where the temp-file path
     * does it after the image has already been written out.
     */
    if (_streaming) {
        if (!prepareDeviceForImage()) return;

        emit dfuProgress(80, tr("Sending image to device (this may take several minutes)..."));
        if (QUrl(QString::fromUtf8(_url)).isLocalFile())
            emit preparationStatusUpdate(tr("Reading image from cache/local file (no download needed)..."));
        DownloadExtractThread::run();
        waitForExtractThread();
        if (_cancelled) { closeStream(); emit error(tr("Cancelled")); return; }
        if (_streamFailed) { closeStream(); return; }   /* already reported */
        if (!_successful) { closeStream(); return; }

        const bool finished = _activeDfu && _activeDfu->finishStream();
        const QString finishError = _activeDfu ? _activeDfu->lastError() : tr("transfer was not open");
        closeStream();
        if (!finished) {
            emit error(tr("DFU transfer failed at the end of the image: %1").arg(finishError));
            return;
        }
    } else {
        if (QUrl(QString::fromUtf8(_url)).isLocalFile())
            emit dfuProgress(5, tr("Reading image from cache/local file (no download needed)..."));
        else
            emit dfuProgress(5, tr("Downloading image..."));
        DownloadExtractThread::run();
        waitForExtractThread();
        if (!_successful) return;
        if (_cancelled) { emit error(tr("Cancelled")); return; }
    }

    if (!_streaming && (!_geminit.isEmpty() || !_config.isEmpty() || !_cmdline.isEmpty() || !_firstrun.isEmpty() || !_cloudinit.isEmpty())) {
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

    if (!_streaming) {
        if (!prepareDeviceForImage()) return;

        emit dfuProgress(80, tr("Sending image to device (this may take several minutes)..."));
        if (!sendImageToRawemmc()) return;
    }

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

void DfuThread::_onWriteError()
{
    if (!_streaming) {
        DownloadExtractThread::_onWriteError();
        return;
    }
    if (_cancelled)
        return;
    /* "Error writing file to disk" would be wrong here: nothing was written to
       a disk, the USB transfer failed. */
    _onDownloadError(tr("DFU transfer failed: %1")
                     .arg(_streamError.isEmpty() ? tr("unknown error") : _streamError));
}

/* Release the USB device and the libusb context the streaming transfer holds.
   The temp-file path does this inside runDfu(); the streaming path keeps the
   wrapper alive across the whole download, so it is closed here instead. */
void DfuThread::closeStream()
{
    if (!_activeDfu)
        return;
    DfuWrapper *dfu = _activeDfu;
    _activeDfu = nullptr;
    dfu->cleanup();
    delete dfu;
}

/* Everything that has to happen before any image data can be sent: get the
   bootloader files, make sure Windows has a driver bound, push the bootloader
   stages and give U-Boot a moment to come up. */
bool DfuThread::prepareDeviceForImage()
{
    emit dfuProgress(38, tr("Fetching bootloader files..."));
    if (!fetchBootloaderFiles()) return false;
    if (_cancelled) { emit error(tr("Cancelled")); return false; }

    /* A missing Windows driver makes every stage below fail after its own
       retries, so check for it once here rather than letting the user wait out
       three rounds of that and then read a libusb error. A device that is not
       attached yet is not an error: the retry loops wait for it on purpose. */
    const QString driverHint = DfuDriver::missingDriverHint(DfuWrapper::TI_VENDOR_ID,
                                                            DfuWrapper::TI_PRODUCT_ID);
    if (!driverHint.isEmpty()) {
        emit error(driverHint);
        return false;
    }

    emit dfuProgress(45, tr("Sending bootloader files..."));
    if (!sendBootloaderFiles()) return false;
    if (_cancelled) { emit error(tr("Cancelled")); return false; }

    emit dfuProgress(77, tr("Waiting for device to enter DFU mode..."));
    QThread::sleep(3);
    if (_cancelled) { emit error(tr("Cancelled")); return false; }

    return true;
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
