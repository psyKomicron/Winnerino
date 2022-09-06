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

    inline winrt::hstring format_size(double* size)
    {
        if (*size >= 0x10000000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x10000000000);
            *size /= (double)100;
            return L" Tb";
        }

        if (*size >= 0x40000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x40000000);
            *size /= (double)100;
            return L" Gb";
        }

        if (*size >= 0x100000)
        {
            *size *= (double)100;
            *size = round(*size / 0x100000);
            *size /= (double)100;
            return L" Mb";
        }

        if (*size >= 0x400)
        {
            *size *= (double)100;
            *size = round(*size / 0x400);
            *size /= (double)100;
            return L" Kb";
        }

        return L" b";
    };

    inline winrt::hstring format_size(double* size, uint16_t const& decimals)
    {
        double multiplicator = pow(10.0, decimals);

        if (*size >= 0x10000000000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x10000000000);
            *size /= multiplicator;
            return L" Tb";
        }

        if (*size >= 0x40000000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x40000000);
            *size /= multiplicator;
            return L" Gb";
        }

        if (*size >= 0x100000)
        {
            *size *= multiplicator;
            *size = round(*size / 0x100000);
            *size /= multiplicator;
            return L" Mb";
        }
        
        if (*size >= 0x400)
        {
            *size *= multiplicator;
            *size = round(*size / 0x400);
            *size /= multiplicator;
            return L" Kb";
        }

        return L" b";
    }

    inline winrt::hstring format_size(double size, uint16_t const& decimals)
    {
        double multiplicator = pow(10, decimals);

        if (size >= 0x10000000000)
        {
            size *= multiplicator;
            size = round(size / 0x10000000000);
            size /= multiplicator;
            return winrt::to_hstring(size) + L" Tb";
        }

        if (size >= 0x40000000)
        {
            size *= multiplicator;
            size = round(size / 0x40000000);
            size /= multiplicator;
            return winrt::to_hstring(size) + L" Gb";
        }

        if (size >= 0x100000)
        {
            size *= multiplicator;
            size = round(size / 0x100000);
            size /= multiplicator;
            return winrt::to_hstring(size) + L" Mb";
        }

        if (size >= 0x400)
        {
            size *= multiplicator;
            size = round(size / 0x400);
            size /= multiplicator;
            return winrt::to_hstring(size) + L" Kb";
        }

        return winrt::to_hstring(size) + L" b";
    };

    inline int64_t convert_large_uint(ULARGE_INTEGER const& largeUInt)
    {
        return (static_cast<int64_t>(largeUInt.HighPart) << 32) | largeUInt.LowPart;
    };

    inline int64_t convert_large_uint(DWORD const& highPart, DWORD const& lowPart)
    {
        return (static_cast<int64_t>(highPart) << 32) | lowPart;
    };
}