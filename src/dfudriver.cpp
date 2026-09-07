/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 T3 Gemstone
 */

#include "dfudriver.h"
#include <QCoreApplication>

#ifdef Q_OS_WIN

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>   /* MAX_DEVICE_ID_LEN; setupapi.h does not pull this in */
#include <QVarLengthArray>

namespace {

/* The drivers libusb's Windows backend can talk to, from winusbx_driver_names
   in libusb's windows_winusb.c. Checking only for WinUSB would wrongly tell a
   user who bound libusbK that no driver is installed. libusb compares these
   case-insensitively, so we do too. */
const char *const kUsableDrivers[] = { "WinUSB", "libusbK", "libusb0" };

bool isUsableDriver(const QString &driver)
{
    for (const char *usable : kUsableDrivers)
        if (driver.compare(QLatin1String(usable), Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

/* Reads a string device property. Returns an empty string when the property is
   absent, which is the normal case for SPDRP_SERVICE on an unbound device. */
QString deviceProperty(HDEVINFO devInfo, SP_DEVINFO_DATA *devData, DWORD property)
{
    DWORD required = 0;
    SetupDiGetDeviceRegistryPropertyW(devInfo, devData, property, nullptr,
                                      nullptr, 0, &required);
    if (required == 0)
        return QString();

    QVarLengthArray<wchar_t, 256> buf(required / sizeof(wchar_t) + 1);
    if (!SetupDiGetDeviceRegistryPropertyW(devInfo, devData, property, nullptr,
                                           reinterpret_cast<PBYTE>(buf.data()),
                                           required, nullptr))
        return QString();

    buf[buf.size() - 1] = L'\0';
    return QString::fromWCharArray(buf.data());
}

} // namespace

DfuDriver::State DfuDriver::probe(quint16 vendorId, quint16 productId, QString *boundDriver)
{
    const QString needle = QString::asprintf("VID_%04X&PID_%04X", vendorId, productId);

    HDEVINFO devInfo = SetupDiGetClassDevsW(nullptr, L"USB", nullptr,
                                            DIGCF_ALLCLASSES | DIGCF_PRESENT);
    if (devInfo == INVALID_HANDLE_VALUE)
        return NoDevice;

    State state = NoDevice;
    QString driverName;

    SP_DEVINFO_DATA devData;
    devData.cbSize = sizeof(devData);
    for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devData); i++) {
        wchar_t instanceId[MAX_DEVICE_ID_LEN];
        if (!SetupDiGetDeviceInstanceIdW(devInfo, &devData, instanceId,
                                         ARRAYSIZE(instanceId), nullptr))
            continue;
        if (!QString::fromWCharArray(instanceId).contains(needle, Qt::CaseInsensitive))
            continue;

        const QString driver = deviceProperty(devInfo, &devData, SPDRP_SERVICE);

        /* A composite device appears as a parent node plus one node per
           interface, where the parent carries usbccgp and the interfaces carry
           the real driver. Any node with a driver libusb understands means the
           device is reachable, so keep looking until one turns up. */
        if (isUsableDriver(driver)) {
            state = Ready;
            driverName = driver;
            break;
        }
        if (state == NoDevice) {
            state = NeedsDriver;
            driverName = driver;
        }
    }

    SetupDiDestroyDeviceInfoList(devInfo);

    if (boundDriver)
        *boundDriver = driverName;
    return state;
}

#else  // !Q_OS_WIN

DfuDriver::State DfuDriver::probe(quint16, quint16, QString *boundDriver)
{
    if (boundDriver)
        boundDriver->clear();
    return Unknown;
}

#endif // Q_OS_WIN

QString DfuDriver::missingDriverHint(quint16 vendorId, quint16 productId)
{
    QString boundDriver;
    if (probe(vendorId, productId, &boundDriver) != NeedsDriver)
        return QString();

    QString hint = QCoreApplication::translate("DfuDriver",
        "Windows cannot reach the board because no suitable USB driver is bound "
        "to its DFU interface.<br><br>"
        "Install the WinUSB driver for <b>USB\\VID_%1&amp;PID_%2</b> using Zadig, "
        "then retry. The board does not need to be power cycled for this.")
        .arg(QString::asprintf("%04X", vendorId), QString::asprintf("%04X", productId));

    if (!boundDriver.isEmpty())
        hint += QCoreApplication::translate("DfuDriver",
            "<br><br>The interface is currently claimed by the <b>%1</b> driver.")
            .arg(boundDriver);

    return hint;
}
