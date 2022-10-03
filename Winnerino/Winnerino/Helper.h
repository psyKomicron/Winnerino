#pragma once
#include "pch.h"
#include "shlwapi.h"
#include <vector>

namespace Winnerino
{
    inline void list_local_drives(std::vector<winrt::hstring>* driveList)
    {
        WCHAR drives[512]{};
        WCHAR* pointer = drives;

        if (GetLogicalDriveStrings(512, drives))
        {
            while (1)
            {
                if (*pointer == NULL)
                {
                    break;
                }
                driveList->push_back(winrt::hstring{ pointer });
                while (*pointer++);
            }
        }
    };

    inline winrt::hstring format_size(double* size, uint16_t const& decimals)
    {
        static const winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceLoader resLoader{};

        double multiplicator = pow(10.0, decimals);

        if (*size >= 0x10000000000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x10000000000);
            *size /= multiplicator;
            return resLoader.GetString(L"TerabytesFileSizeExtension");
        }

        if (*size >= 0x40000000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x40000000);
            *size /= multiplicator;
            return resLoader.GetString(L"GigabytesFileSizeExtension");
        }

        if (*size >= 0x100000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x100000);
            *size /= multiplicator;
            return resLoader.GetString(L"MegabytesFileSizeExtension");
        }

        if (*size >= 0x400)
        {
            *size *= multiplicator;
            *size = round(*size / 0x400);
            *size /= multiplicator;
            return resLoader.GetString(L"KilobytesFileSizeExtension");
        }

        return resLoader.GetString(L"BytesFileSizeExtension");
    };

    inline winrt::hstring format_size(double* size)
    {
        return format_size(size, 3);
    };

    inline winrt::hstring format_size(double size, uint16_t const& decimals)
    {
        return format_size(&size, decimals);
    };

    inline int64_t convert_large_uint(ULARGE_INTEGER const& largeUInt)
    {
        return (static_cast<int64_t>(largeUInt.HighPart) << 32) | largeUInt.LowPart;
    };

    inline int64_t convert_large_uint(DWORD const& highPart, DWORD const& lowPart)
    {
        return (static_cast<int64_t>(highPart) << 32) | lowPart;
    };

    inline winrt::Windows::Storage::ApplicationDataContainer get_or_create_container(winrt::hstring const& containerName)
    {
        winrt::Windows::Storage::ApplicationDataContainer container = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Containers().TryLookup(containerName);
        if (!container)
        {
            return winrt::Windows::Storage::ApplicationData::Current().LocalSettings().CreateContainer(L"Explorer",
                winrt::Windows::Storage::ApplicationDataCreateDisposition::Always);
        }
        return container;
    }
}