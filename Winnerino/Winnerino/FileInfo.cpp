#include "pch.h"
#include "FileInfo.h"

#include <fileapi.h>
#include <Shlwapi.h>
#include <chrono>
#include "Helper.h"

namespace Winnerino::Storage
{
    FileInfo::FileInfo(WIN32_FIND_DATA* findData, winrt::hstring _parentPath)
    {
        attributes = findData->dwFileAttributes;
        size = Winnerino::convert_large_uint(findData->nFileSizeHigh, findData->nFileSizeLow);
        fileName = winrt::to_hstring(findData->cFileName);
        parentPath = winrt::hstring(_parentPath);
        lastWrite = winrt::clock::from_FILETIME(findData->ftLastWriteTime);

        WCHAR filePathWstr[ALTERNATE_MAX_PATH];
        if (PathCombine(filePathWstr, parentPath.c_str(), fileName.c_str()))
        {
            filePath = winrt::to_hstring(filePathWstr);
        }
    }

    FileInfo::~FileInfo()
    {
        attributes = 0;
        size = 0;
    }

    bool FileInfo::IsDirectory() const
    {
        return attributes & FILE_ATTRIBUTE_DIRECTORY;
    }

    bool FileInfo::IsSystemFile() const
    {
        return attributes & FILE_ATTRIBUTE_SYSTEM;
    }

    winrt::Windows::Foundation::DateTime FileInfo::LastWrite() const
    {
        return lastWrite;
    }

    winrt::hstring FileInfo::Name() const
    {
        return fileName;
    }

    winrt::hstring FileInfo::Path() const
    {
        return filePath;
    }

    winrt::hstring FileInfo::ParentPath() const
    {
        return parentPath;
    }

    DWORD FileInfo::Attributes() const
    {
        return attributes;
    }

    uint64_t FileInfo::Size() const
    {
        return size;
    }
}