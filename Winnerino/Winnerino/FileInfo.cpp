#include "pch.h"
#include "FileInfo.h"

#include <fileapi.h>
#include <Shlwapi.h>
#include "Helper.h"

namespace Winnerino::Storage
{
    FileInfo::FileInfo(WIN32_FIND_DATA* findData, winrt::hstring path)
    {
        fileName = winrt::to_hstring(findData->cFileName);
        parentPath = winrt::hstring{ path };
        WCHAR filePathWstr[ALTERNATE_MAX_PATH];
        if (PathCombine(filePathWstr, parentPath.c_str(), fileName.c_str()))
        {
            filePath = winrt::to_hstring(filePathWstr);
        }
        size = Winnerino::convert_large_uint(findData->nFileSizeHigh, findData->nFileSizeLow);
        attributes = findData->dwFileAttributes;
    }
}