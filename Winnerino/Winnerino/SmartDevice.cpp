#include "pch.h"
#include "SmartDevice.h"
#include <winioctl.h>

namespace Winnerino::Devices
{
    SmartDevice::SmartDevice(winrt::hstring const& deviceName)
    {
        this->deviceName = deviceName;
    }

    SmartInfo SmartDevice::GetInfo()
    {
        HANDLE hPhysicalDriveIOCTL = INVALID_HANDLE_VALUE;

        hPhysicalDriveIOCTL = CreateFile(deviceName.c_str(), desiredAccess, shareMode, securityAttributes, creationDisposition, flags, NULL);
        if (hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE)
        {
            GETVERSIONINPARAMS getVersionInParams{};
            DWORD bytesReturned = 0;

            winrt::hstring name;
            if (DeviceIoControl(hPhysicalDriveIOCTL, SMART_GET_VERSION, NULL, 0, &getVersionInParams, sizeof(GETVERSIONINPARAMS), &bytesReturned, NULL))
            {
                uint64_t commandSize = sizeof(SENDCMDINPARAMS) + IDENTIFY_BUFFER_SIZE;
                std::unique_ptr<SENDCMDINPARAMS> command{ new SENDCMDINPARAMS[commandSize] };
                command->irDriveRegs.bCommandReg = ID_CMD;
                
                if (DeviceIoControl(hPhysicalDriveIOCTL, SMART_RCV_DRIVE_DATA, command.get(), sizeof(SENDCMDINPARAMS), command.get(), commandSize, &bytesReturned, NULL))
                {
                    DWORD diskData[256]{ 0 };
                    return SmartInfo(diskData);
                }
            }

            CloseHandle(hPhysicalDriveIOCTL);
        }

        return SmartInfo();
    }
}