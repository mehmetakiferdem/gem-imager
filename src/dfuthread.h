/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2024 Raspberry Pi Ltd
 */

#ifndef DFUTHREAD_H
#define DFUTHREAD_H

#include "downloadextractthread.h"
#include <QTemporaryFile>

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
     * front. Not usable together with image customization, which has to seek
     * around inside the image.
     */
    void setStreamImageSize(qint64 imageSize);

signals:
    void dfuProgress(int percentage, QString statusMsg);

protected:
    void run() override;
    bool _openAndPrepareDevice() override;
    size_t _writeFile(const char *buf, size_t len) override;
    void _onWriteError() override;

private:
    QString _bootloaderFiles[3];
    QByteArray _expectedTiboot3Hash;
    QByteArray _expectedTisplHash;
    QByteArray _expectedUbootHash;
    QTemporaryFile *_tempImageFile;
    QString _tempImagePath;
    QString _tempDir;

    class DfuWrapper *_activeDfu;
    bool _streaming = false;
    qint64 _streamImageSize = 0;
    bool _streamFailed = false;
    bool runDfu(const QString &altSetting, const QString &filePath, bool resetAfter);
    bool prepareDeviceForImage();
    bool openStreamToRawemmc();
    void closeStream();
    QString _streamError;
    bool fetchBootloaderFiles();
    bool sendBootloaderFiles();
    bool sendImageToRawemmc();
    void onStreamProgress(qint64 bytesSent, qint64 totalBytes);
};

#endif // DFUTHREAD_H
