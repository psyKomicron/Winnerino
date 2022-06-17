#pragma once
namespace Winnerino::Devices
{
    class SmartInfo
    {
    public:
        SmartInfo() = default;
        SmartInfo(DWORD data[256]);

        winrt::hstring Info()
        {
            return message;
        };
        winrt::hstring DeviceName()
        {
            return deviceName;
        };

    private:
        winrt::hstring message;
        winrt::hstring deviceName;
    };
};

