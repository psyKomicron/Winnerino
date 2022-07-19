#include "pch.h"
#include "DirectoryEnumerator.h"

#include <Shlwapi.h>

using namespace winrt;
using namespace std;

namespace Winnerino::Storage
{
    vector<hstring>* DirectoryEnumerator::Enumerate(hstring const& toEnumerate)
    {
        CheckArguments(toEnumerate);
        hstring path = toEnumerate.ends_with(L"\\") ? toEnumerate : toEnumerate + L"\\";

        vector<hstring>* pathes = nullptr;
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            pathes = new vector<hstring>();
            do
            {
                hstring filePath{ findData.cFileName };
                if (filePath != L"." && filePath != L"..")
                {
                    pathes->push_back(path + filePath);
                }
            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
        }

        return pathes;
    }

    vector<hstring>* DirectoryEnumerator::EnumerateDirectories(winrt::hstring const& toEnumerate)
    {
        CheckArguments(toEnumerate);
        hstring path = toEnumerate.ends_with(L"\\") ? toEnumerate : toEnumerate + L"\\";

        vector<hstring>* pathes = nullptr;
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            pathes = new vector<hstring>();
            do
            {
                hstring filePath{ findData.cFileName };
                if ((includeSpecials || (filePath != L"." && filePath != L"..")) && findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    pathes->push_back(path + filePath);
                }
            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
        }

        return pathes;
    }
    
    vector<hstring>* DirectoryEnumerator::EnumerateFiles(winrt::hstring const& toEnumerate)
    {
        CheckArguments(toEnumerate);
        hstring path = toEnumerate.ends_with(L"\\") ? toEnumerate : toEnumerate + L"\\";

        vector<hstring>* pathes = nullptr;
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            pathes = new vector<hstring>();
            do
            {
                hstring filePath{ findData.cFileName };
                if ((includeSpecials || (filePath != L"." && filePath != L"..")) && (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    pathes->push_back(path + filePath);
                }
            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
        }

        return pathes;
    }

    vector<hstring>* DirectoryEnumerator::EnumerateDrives()
    {
        WCHAR drives[512]{};
        WCHAR* pointer = drives;
        vector<hstring>* driveList = nullptr;

        if (GetLogicalDriveStrings(512, drives))
        {
            driveList = new vector<hstring>();
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

        return driveList;
    }

    vector<FileInfo>* DirectoryEnumerator::GetFiles(winrt::hstring const& directory)
    {
        vector<FileInfo>* files = nullptr;
        hstring toEnumerate = directory.ends_with(L"\\") ? directory : directory + L"\\";
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            files = new vector<FileInfo>();
            do
            {
                // is file
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    files->push_back(FileInfo(&findData, toEnumerate));
                }
            } while (FindNextFile(findHandle, &findData));

            FindClose(findHandle);
        }
        return files;
    }

    inline void DirectoryEnumerator::CheckArguments(hstring const& path)
    {
        if (path.empty())
        {
            throw hresult_invalid_argument(L"Path is empty");
        }
        else if (path == L"." || path == L"..")
        {
            throw hresult_invalid_argument(L"Path is not valid");
        }
    }
}