/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2024 Raspberry Pi Ltd
 */

#include "dfuwrapper.h"
#include <QDebug>
#include <QFile>
#include <QThread>
#include <string.h>
#include <stdlib.h>

extern "C" {
#include <libusb.h>
#include "dfu.h"
#include "dfu_file.h"
#include "dfu_load.h"
#include "dfu_util.h"
}

// dfu-util global state
extern "C" {
    int verbose = 0;
    struct dfu_if *dfu_root = nullptr;
    char *match_path = nullptr;
    int match_vendor = -1;
    int match_product = -1;
    int match_vendor_dfu = -1;
    int match_product_dfu = -1;
    int match_config_index = -1;
    int match_iface_index = -1;
    int match_iface_alt_index = -1;
    int match_devnum = -1;
    const char *match_iface_alt_name = nullptr;
    const char *match_serial = nullptr;
    const char *match_serial_dfu = nullptr;
}

DfuWrapper::DfuWrapper(QObject *parent)
    : QObject(parent), usbContext(nullptr), dfuDevice(nullptr), initialized(false), _cancelled(0)
{
    // Mirror status messages to the terminal; the GUI is not the only consumer
    // of progress information when debugging DFU transfers.
    connect(this, &DfuWrapper::statusMessage,
            [](const QString &msg) { qDebug() << "DFU:" << msg; });
}

DfuWrapper::~DfuWrapper()
{
    cleanup();
}

void DfuWrapper::cancel()
{
    _cancelled.storeRelease(1);
}

bool DfuWrapper::isCancelled() const
{
    return _cancelled.loadAcquire() != 0;
}

void DfuWrapper::setError(const QString &msg)
{
    _lastError = msg;
    emit statusMessage(QString("error: %1").arg(msg));
}

// Checks the bus with a private libusb context so it can be called after any
// DfuWrapper instance has been destroyed.
bool DfuWrapper::isDevicePresent(int vendorId, int productId)
{
    libusb_context *ctx = nullptr;
    if (libusb_init(&ctx) < 0)
        return false;

    libusb_device **list = nullptr;
    ssize_t count = libusb_get_device_list(ctx, &list);
    bool present = false;
    for (ssize_t i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) == 0
                && desc.idVendor == vendorId && desc.idProduct == productId) {
            present = true;
            break;
        }
    }
    if (count >= 0)
        libusb_free_device_list(list, 1);
    libusb_exit(ctx);
    return present;
}

bool DfuWrapper::initialize()
{
    if (initialized)
        return true;

    int ret = libusb_init(&usbContext);
    if (ret < 0) {
        setError(QString("Failed to initialize libusb: %1").arg(libusb_error_name(ret)));
        return false;
    }

    initialized = true;
    return true;
}

bool DfuWrapper::findDevice(int vendorId, int productId, const QString &altSettingName)
{
    if (!initialized) {
        setError("DFU not initialized");
        return false;
    }

    match_vendor  = vendorId;
    match_product = productId;

    _altNameBytes = altSettingName.toUtf8();
    match_iface_alt_name = altSettingName.isEmpty() ? nullptr : _altNameBytes.constData();

    for (int attempt = 0; attempt < 15; attempt++) {
        if (isCancelled()) {
            setError("Device search cancelled by user");
            return false;
        }
        if (attempt > 0) {
            qDebug() << "Retry" << attempt << "searching for DFU device...";
            QThread::sleep(1);
        }
        disconnect_devices();
        probe_devices(usbContext);
        if (dfu_root)
            break;
    }

    if (!dfu_root) {
        setError(QString("No DFU device found (VID:0x%1 PID:0x%2 alt:%3) after 15 retries")
                .arg(vendorId, 4, 16, QChar('0'))
                .arg(productId, 4, 16, QChar('0'))
                .arg(altSettingName));
        return false;
    }

    dfuDevice = dfu_root;

    int ret = libusb_open(dfuDevice->dev, &dfuDevice->dev_handle);
    if (ret < 0) {
        setError(QString("Failed to open DFU device: %1").arg(libusb_error_name(ret)));
        return false;
    }

    emit statusMessage(QString("Found DFU device: %1:%2 alt:%3")
                      .arg(dfuDevice->vendor, 4, 16, QChar('0'))
                      .arg(dfuDevice->product, 4, 16, QChar('0'))
                      .arg(altSettingName));
    return true;
}

// Claim the USB interface and put the device in a known good state.
// Caller is responsible for releasing the interface on success.
bool DfuWrapper::claimInterface()
{
    int ret = libusb_claim_interface(dfuDevice->dev_handle, dfuDevice->interface);
    if (ret < 0) {
        setError(QString("Cannot claim interface: %1").arg(libusb_error_name(ret)));
        return false;
    }

    if (dfuDevice->flags & DFU_IFF_ALT) {
        ret = libusb_set_interface_alt_setting(dfuDevice->dev_handle,
                                               dfuDevice->interface,
                                               dfuDevice->altsetting);
        if (ret < 0) {
            setError(QString("Cannot set alternate interface: %1").arg(libusb_error_name(ret)));
            libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
            return false;
        }
    }

    struct dfu_status status;
    ret = dfu_get_status(dfuDevice, &status);
    if (ret < 0) {
        setError(QString("Error getting DFU status: %1").arg(libusb_error_name(ret)));
        libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
        return false;
    }

    if (status.bState == DFU_STATE_dfuERROR) {
        dfu_clear_status(dfuDevice->dev_handle, dfuDevice->interface);
        dfu_get_status(dfuDevice, &status);
    }

    if (status.bState == DFU_STATE_dfuDNLOAD_IDLE || status.bState == DFU_STATE_dfuUPLOAD_IDLE)
        dfu_abort(dfuDevice->dev_handle, dfuDevice->interface);

    return true;
}

bool DfuWrapper::downloadFile(const QString &filePath, bool resetAfter)
{
    if (!dfuDevice || !dfuDevice->dev_handle) {
        setError("No DFU device");
        return false;
    }

    if (!claimInterface())
        return false;

    struct dfu_file file;
    memset(&file, 0, sizeof(file));
    QByteArray filePathBytes = filePath.toUtf8();
    file.name = filePathBytes.constData();
    dfu_load_file(&file, MAYBE_SUFFIX, NO_PREFIX);

    if (!file.firmware || file.size.total == 0) {
        setError(QString("Failed to load file: %1").arg(filePath));
        libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
        if (file.firmware) free(file.firmware);
        return false;
    }

    emit statusMessage(QString("Downloading %1...").arg(filePath));
    int ret = dfuload_do_dnload(dfuDevice, getTransferSize(), &file);
    free(file.firmware);
    libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);

    if (ret < 0) {
        if (!resetAfter) {
            setError(QString("Download failed: %1").arg(libusb_error_name(ret)));
            return false;
        }
        // With resetAfter, I/O errors are expected (device resets after each bootloader stage).
        // Only reject errors that indicate a real problem unrelated to disconnect.
        switch (ret) {
        case LIBUSB_ERROR_INVALID_PARAM:
        case LIBUSB_ERROR_ACCESS:
        case LIBUSB_ERROR_BUSY:
        case LIBUSB_ERROR_OVERFLOW:
        case LIBUSB_ERROR_NO_MEM:
        case LIBUSB_ERROR_NOT_SUPPORTED:
            setError(QString("Download failed: %1").arg(libusb_error_name(ret)));
            return false;
        default:
            break; // IO/NO_DEVICE/PIPE/TIMEOUT/etc: expected disconnect
        }
    }

    if (resetAfter) {
        dfu_detach(dfuDevice->dev_handle, dfuDevice->interface, 1000);
        libusb_reset_device(dfuDevice->dev_handle);
        libusb_close(dfuDevice->dev_handle);
        dfuDevice->dev_handle = nullptr;
    }

    return true;
}

/*
 * The device is ready for the next block once it reports DNLOAD_IDLE (or has
 * gone into an error state we handle above). Status polls fail transiently
 * while U-Boot flushes to eMMC, so a few failures in a row are tolerated;
 * a definitive disconnect is not.
 */
bool DfuWrapper::waitForDeviceIdle(struct dfu_status *dst)
{
    int statusRetries = 0;
    for (;;) {
        int ret = dfu_get_status(dfuDevice, dst);
        if (ret < 0) {
            if (ret != LIBUSB_ERROR_NO_DEVICE && ++statusRetries <= 3) {
                QThread::msleep(100);
                continue;
            }
            setError(QString("Status poll error: %1").arg(libusb_error_name(ret)));
            return false;
        }
        statusRetries = 0;
        if (dst->bState == DFU_STATE_dfuDNLOAD_IDLE || dst->bState == DFU_STATE_dfuERROR)
            return true;
        QThread::msleep(dst->bwPollTimeout > 0 ? dst->bwPollTimeout : 1);
    }
}

bool DfuWrapper::beginStream(qint64 totalBytes)
{
    if (!dfuDevice || !dfuDevice->dev_handle) {
        setError("No DFU device");
        return false;
    }
    if (totalBytes <= 0) {
        setError("Image size is not known, cannot transfer");
        return false;
    }
    if (!claimInterface())
        return false;

    _streamXferSize = getTransferSize();
    if (_streamXferSize <= 0)
        _streamXferSize = 4096;
    _streamTotal = totalBytes;
    _streamSent = 0;
    _streamTransaction = 0;
    _streamRestarted = false;
    _streamPending.clear();
    _streamActive = true;

    // U-Boot's rawemmc alt-setting can take minutes to flush its DFU buffer to eMMC
    dfu_set_timeout(300000);

    emit statusMessage(QString("Streaming %1 MB to device (this may take several minutes)...")
                       .arg(_streamTotal / 1024 / 1024));
    return true;
}

bool DfuWrapper::streamChunk(const char *data, qint64 len)
{
    if (!_streamActive) {
        setError("Transfer has not been started");
        return false;
    }

    /*
     * The caller's chunk size has nothing to do with the device's, so buffer
     * and hand over exactly wTransferSize at a time. The tail of a chunk stays
     * in _streamPending until enough has arrived, or until finishStream()
     * flushes it as the last (short) block.
     */
    _streamPending.append(data, len);

    while (_streamPending.size() >= _streamXferSize
           || (_streamSent + _streamPending.size() == _streamTotal && !_streamPending.isEmpty()))
    {
        const qint64 thisBlock = qMin<qint64>(_streamXferSize, _streamPending.size());

        if (isCancelled()) {
            setError("Transfer cancelled by user");
            return false;
        }
        if (_streamSent + thisBlock > _streamTotal) {
            setError(QString("Transfer overran the declared image size of %1 bytes")
                     .arg(_streamTotal));
            return false;
        }

        int ret = dfu_download(dfuDevice->dev_handle, dfuDevice->interface,
                               (unsigned short)thisBlock, _streamTransaction++,
                               (unsigned char *)_streamPending.data());
        if (ret < 0) {
            setError(QString("Download error: %1").arg(libusb_error_name(ret)));
            return false;
        }

        struct dfu_status dst;
        if (!waitForDeviceIdle(&dst))
            return false;

        if (dst.bStatus != DFU_STATUS_OK) {
            /* A stale DFU session left on the device by an interrupted transfer
               rejects our first block with a sequence-number mismatch
               ("dfu_write: Wrong sequence number!" on the device console) and
               cleans itself up in the process. Clear the error state and send
               this block again; DFU_ABORT alone does not reset U-Boot's block
               counter, so this rejection is the only reliable reset. Nothing
               has been accepted yet at this point, so the block we still hold
               is all that needs resending. */
            if (_streamTransaction == 1 && !_streamRestarted) {
                _streamRestarted = true;
                dfu_clear_status(dfuDevice->dev_handle, dfuDevice->interface);
                emit statusMessage("Device had a stale DFU session, restarting transfer from the beginning...");
                _streamTransaction = 0;
                continue;
            }
            setError(QString("DFU device error: state=%1 status=%2").arg(dst.bState).arg(dst.bStatus));
            return false;
        }

        _streamPending.remove(0, thisBlock);
        _streamSent += thisBlock;

        emit streamProgress(_streamSent, _streamTotal);
        if ((_streamSent % (10LL * 1024 * 1024)) < _streamXferSize || _streamSent == _streamTotal)
            emit statusMessage(QString("Transferred %1 / %2 MB...")
                               .arg(_streamSent / 1024 / 1024).arg(_streamTotal / 1024 / 1024));
    }

    return true;
}

bool DfuWrapper::finishStream()
{
    if (!_streamActive) {
        setError("Transfer has not been started");
        return false;
    }
    _streamActive = false;

    bool ok = true;

    /* Anything still buffered is a short final block */
    if (!_streamPending.isEmpty())
    {
        const qint64 thisBlock = _streamPending.size();
        int ret = dfu_download(dfuDevice->dev_handle, dfuDevice->interface,
                               (unsigned short)thisBlock, _streamTransaction++,
                               (unsigned char *)_streamPending.data());
        if (ret < 0) {
            setError(QString("Download error: %1").arg(libusb_error_name(ret)));
            ok = false;
        } else {
            struct dfu_status dst;
            if (!waitForDeviceIdle(&dst)) {
                ok = false;
            } else if (dst.bStatus != DFU_STATUS_OK) {
                setError(QString("DFU device error: state=%1 status=%2").arg(dst.bState).arg(dst.bStatus));
                ok = false;
            } else {
                _streamPending.clear();
                _streamSent += thisBlock;
                emit streamProgress(_streamSent, _streamTotal);
            }
        }
    }

    /* Sending the end-of-transfer packet after a short transfer would make the
       device commit a truncated image, so refuse instead. */
    if (ok && _streamSent != _streamTotal) {
        setError(QString("Transfer incomplete: sent %1 of %2 bytes")
                 .arg(_streamSent).arg(_streamTotal));
        ok = false;
    }

    if (ok) {
        // Zero-length packet signals end of transfer
        dfu_download(dfuDevice->dev_handle, dfuDevice->interface, 0, _streamTransaction, nullptr);

        // Wait for manifest phase (final eMMC flush)
        emit statusMessage("Waiting for device to complete writing...");
        struct dfu_status finalStatus;
        bool manifestOk = false;
        for (;;) {
            memset(&finalStatus, 0, sizeof(finalStatus));
            int ret = dfu_get_status(dfuDevice, &finalStatus);
            if (ret < 0) {
                // Device disconnected. This is expected only after a successful
                // manifest (MANIFEST_WAIT_RST or IDLE was seen before disconnect).
                // If we never saw a terminal state, treat as error.
                if (!manifestOk) {
                    setError("Device disconnected unexpectedly during manifest phase");
                    ok = false;
                }
                break;
            }

            if (finalStatus.bState == DFU_STATE_dfuIDLE) {
                manifestOk = true;
                break;
            }
            if (finalStatus.bState == DFU_STATE_dfuMANIFEST_WAIT_RST) {
                manifestOk = true;
                libusb_reset_device(dfuDevice->dev_handle);
                break;
            }
            if (finalStatus.bState == DFU_STATE_dfuMANIFEST) {
                // Still writing — keep polling
                manifestOk = false;
            }
            if (finalStatus.bState == DFU_STATE_dfuERROR) {
                setError(QString("DFU error in manifest phase: status=%1").arg(finalStatus.bStatus));
                ok = false;
                break;
            }

            unsigned int pollMs = finalStatus.bwPollTimeout > 0 ? finalStatus.bwPollTimeout : 100;
            QThread::msleep(pollMs);
        }

        if (ok) {
            // Send DFU_DETACH to trigger U-Boot's board_dfu_complete() callback,
            // which writes boot binaries (tiboot3.bin etc.) to eMMC boot partition.
            emit statusMessage("Triggering eMMC boot partition write...");
            dfu_detach(dfuDevice->dev_handle, dfuDevice->interface, 1000);
        }
    }

    dfu_set_timeout(5000);
    libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
    return ok;
}

/* Drive the push interface above from a file, for callers that already have one */
bool DfuWrapper::downloadFileStreaming(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QString("Failed to open file: %1").arg(filePath));
        return false;
    }
    const qint64 fileSize = file.size();
    if (fileSize == 0) {
        setError("Image file is empty, cannot transfer");
        return false;
    }
    if (!beginStream(fileSize))
        return false;

    QByteArray buf(_streamXferSize, 0);
    qint64 read = 0;
    while (read < fileSize) {
        const qint64 n = file.read(buf.data(), qMin<qint64>(buf.size(), fileSize - read));
        if (n <= 0) {
            setError("File read error during streaming");
            _streamActive = false;
            dfu_set_timeout(5000);
            libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
            return false;
        }
        if (!streamChunk(buf.constData(), n)) {
            _streamActive = false;
            dfu_set_timeout(5000);
            libusb_release_interface(dfuDevice->dev_handle, dfuDevice->interface);
            return false;
        }
        read += n;
    }
    file.close();
    return finishStream();
}

void DfuWrapper::cleanup()
{
    if (dfuDevice && dfuDevice->dev_handle) {
        libusb_close(dfuDevice->dev_handle);
        dfuDevice->dev_handle = nullptr;
    }
    disconnect_devices();
    dfuDevice = nullptr;

    if (usbContext) {
        libusb_exit(usbContext);
        usbContext = nullptr;
    }
    initialized = false;
}

int DfuWrapper::getTransferSize()
{
    if (!dfuDevice)
        return 0;
    int size = dfuDevice->func_dfu.wTransferSize;
    return size > 0 ? size : 1024;
}
