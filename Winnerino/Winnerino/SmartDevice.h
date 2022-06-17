#pragma once
#include "SmartInfo.h"

namespace Winnerino::Devices
{
    class SmartDevice
    {
    public:
        SmartDevice(winrt::hstring const& deviceName);

        SmartInfo GetInfo();

    private:
        const uint16_t MaxDrives = 16;
        winrt::hstring deviceName;
        DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE;
        DWORD shareMode = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;
        LPSECURITY_ATTRIBUTES securityAttributes = NULL;
        DWORD creationDisposition = OPEN_EXISTING;
        DWORD flags = 0;
    };
};

