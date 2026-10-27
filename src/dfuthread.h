/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2024 Raspberry Pi Ltd
 */

#ifndef DFUTHREAD_H
#define DFUTHREAD_H

#include "downloadextractthread.h"
#include <QTemporaryFile>
#include <QMutex>
#include <atomic>

class DfuThread : public DownloadExtractThread
{
    Q_OBJECT
public:
    explicit DfuThread(const QByteArray &url, const QByteArray &localfilename,
                       const QByteArray &expectedHash, const QByteArray &tiboot3Hash, const QByteArray &tisplHash, const QByteArray &ubootHash, QObject *parent = nullptr);
    ~DfuThread();

    bool isImage() override;
    void cancelDownload() override;
    void setTempDirectory(const QString &dir);

    /*
     * Stream the image straight to the device as it is decompressed, instead of
     * extracting it to a temporary file first. Saves needing as much free disk
     * space as the uncompressed image - 17 GB for the 16 GiB Android image.
     * imageSize is the uncompressed length, which the DFU transfer needs up
     * front.
     *
     * With customizeHead set, the start of the image - up to the end of the
     * first partition, where customization writes config.ini and friends - is
     * held back in a small temporary file, customized there, and only then
     * sent. That needs as much free space as the boot partition, not the image.
     */
    void setStreamImageSize(qint64 imageSize, bool customizeHead = false);

signals:
    void dfuProgress(int percentage, QString statusMsg);

protected:
    void run() override;
    bool _openAndPrepareDevice() override;
    size_t _writeFile(const char *buf, size_t len) override;
    void _writeComplete() override;
    void _onWriteError() override;
    void _onDownloadError(const QString &msg) override;
    void _beforeCloseDevice() override;

private:
    QString _bootloaderFiles[3];
    QByteArray _expectedTiboot3Hash;
    QByteArray _expectedTisplHash;
    QByteArray _expectedUbootHash;
    QTemporaryFile *_tempImageFile;
    QString _tempImagePath;
    QString _tempDir;

    class DfuWrapper *_activeDfu;
    /* cancelDownload() runs on the GUI thread while the worker swaps and
       deletes the wrapper; both go through this */
    QMutex _activeDfuMutex;
    void setActiveDfu(class DfuWrapper *dfu);
    bool _streaming = false;
    qint64 _streamImageSize = 0;
    bool _streamFailed = false;
    std::atomic<bool> _errorReported{false};

    /*
     * Mass storage mode: a U-Boot that offers the "ums" alt-setting can expose
     * the eMMC as a USB disk, which is written like an SD card - several times
     * faster than DFU. _ums is set once the disk has been found.
     */
    bool _ums = false;
    qint64 _umsLastReport = 0;
    enum class UmsResult { Unavailable, Ready, Failed };
    UmsResult switchToMassStorage();
    bool waitForBoardToLeaveUsb(bool massStorage);
    bool _streamErrorReported = false;
    bool _headPending = false;
    qint64 _headEnd = 0;
    qint64 _headBytes = 0;
    QString _headPath;
    qint64 bufferHead(const char *buf, qint64 len);
    bool customizeAndSendHead();
    void failStream(const QString &msg);
    bool runDfu(const QString &altSetting, const QString &filePath, bool resetAfter);
    bool prepareDeviceForImage();
    bool openStreamToRawemmc();
    void closeStream();
    QString _streamError;
    bool fetchBootloaderFiles();
    bool takeBootloaderFromImage();
    bool sendBootloaderFiles();
    bool sendImageToRawemmc();
    void onStreamProgress(qint64 bytesSent, qint64 totalBytes);
};

#endif // DFUTHREAD_H
