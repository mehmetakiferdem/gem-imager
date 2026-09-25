/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2024 Raspberry Pi Ltd
 */

#ifndef DFUWRAPPER_H
#define DFUWRAPPER_H

#include <QString>
#include <QObject>
#include <QAtomicInt>

struct dfu_if;
struct libusb_context;

class DfuWrapper : public QObject
{
    Q_OBJECT

public:
    static constexpr int TI_VENDOR_ID  = 0x0451;
    static constexpr int TI_PRODUCT_ID = 0x6165;

    static constexpr const char* ALT_BOOTLOADER = "bootloader";
    static constexpr const char* ALT_TISPL      = "tispl.bin";
    static constexpr const char* ALT_UBOOT      = "u-boot.img";
    static constexpr const char* ALT_RAWEMMC    = "rawemmc";

    explicit DfuWrapper(QObject *parent = nullptr);
    ~DfuWrapper();

    static bool isDevicePresent(int vendorId, int productId);

    bool initialize();
    bool findDevice(int vendorId, int productId, const QString &altSettingName);
    bool downloadFile(const QString &filePath, bool resetAfter = true);
    bool downloadFileStreaming(const QString &filePath);

    /*
     * Push interface for callers that produce the image as they go, so a
     * multi-gigabyte image never has to be written to disk first. beginStream()
     * claims the interface and takes the total size (needed for progress and to
     * know when the transfer is complete), streamChunk() may be called with any
     * chunk size, and finishStream() sends the end-of-transfer packet and waits
     * out the device's final flush. downloadFileStreaming() is these three
     * driven from a file.
     */
    bool beginStream(qint64 totalBytes);
    bool streamChunk(const char *data, qint64 len);
    bool finishStream();

    QString lastError() const { return _lastError; }
    void cancel();
    bool isCancelled() const;
    void cleanup();

signals:
    void statusMessage(QString message);
    void streamProgress(qint64 bytesSent, qint64 totalBytes);

private:
    struct libusb_context *usbContext;
    struct dfu_if *dfuDevice;
    bool initialized;
    QAtomicInt _cancelled;
    QString _lastError;
    QByteArray _altNameBytes;

    int  getTransferSize();
    void setError(const QString &msg);
    bool claimInterface();
    bool waitForDeviceIdle(struct dfu_status *dst);
    bool sendOneBlock(const char *data, qint64 len);

    /* beginStream()/streamChunk()/finishStream() state */
    int _streamXferSize = 0;
    qint64 _streamTotal = 0;
    qint64 _streamSent = 0;
    unsigned short _streamTransaction = 0;
    bool _streamRestarted = false;
    bool _streamActive = false;
    QByteArray _streamPending;
};

#endif // DFUWRAPPER_H
