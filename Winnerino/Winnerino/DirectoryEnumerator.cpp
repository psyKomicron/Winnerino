#include "pch.h"
#include "DirectoryEnumerator.h"

#include <Shlwapi.h>
#include <fileapi.h>

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
            const wchar_t dot[2]{ '.', '\0' };
            const wchar_t dotdot[3]{ '.', '.', '\0' };

            pathes = new vector<hstring>();

            do
            {
                if (wcscmp(findData.cFileName, dot) != 0 && wcscmp(findData.cFileName, dotdot) != 0)
                {
                    pathes->push_back(path + hstring(findData.cFileName));
                }

            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
        }

        return pathes;
    }

    vector<hstring>* DirectoryEnumerator::EnumerateFolders(winrt::hstring const& toEnumerate)
    {
        CheckArguments(toEnumerate);

        hstring path = toEnumerate.ends_with(L"\\") ? toEnumerate : toEnumerate + L"\\";

        vector<hstring>* pathes = nullptr;

        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            const wchar_t dot[2]{ '.', '\0' };
            const wchar_t dotdot[3]{ '.', '.', '\0' };

            pathes = new vector<hstring>();
            do
            {
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                    (includeSpecials || (wcscmp(findData.cFileName, dot) != 0 && wcscmp(findData.cFileName, dotdot) != 0)))
                {
                    pathes->push_back(path + hstring(findData.cFileName));
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
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 && (includeSpecials || (filePath != L"." && filePath != L"..")))
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

    std::vector<::Winnerino::Storage::DriveInfo>* DirectoryEnumerator::GetDrives()
    {
        WCHAR drives[512]{};
        WCHAR* pointer = drives;
        vector<DriveInfo>* driveList = nullptr;

        if (GetLogicalDriveStrings(512, drives))
        {
            driveList = new vector<DriveInfo>();
            uint16_t iterator = 1;
            // TODO: Remove while(1) loop
            while (iterator < 500)
            {
                if (*pointer == NULL)
                {
                    break;
                }

                WCHAR volumeName[MAX_PATH + 1];
                if (SUCCEEDED(GetVolumeInformation(pointer, volumeName, MAX_PATH + 1, NULL, NULL, NULL, NULL, NULL)))
                {
                    driveList->push_back(DriveInfo(to_hstring(pointer), to_hstring(volumeName), PathFileExists(pointer)));
                }
                else
                {
                    driveList->push_back(DriveInfo(to_hstring(pointer), L"", PathFileExists(pointer)));
                }

                while (*pointer++);
            }
        }

        return driveList;
    }

    vector<FileInfo>* DirectoryEnumerator::GetFiles(winrt::hstring const& directory)
    {
        CheckArguments(directory);

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

    bool DirectoryEnumerator::GetFiles(winrt::hstring const& directory, std::vector<::Winnerino::Storage::FileInfo>* files)
    {
        CheckArguments(directory);

        hstring toEnumerate = directory.ends_with(L"\\") ? directory : directory + L"\\";

        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            if (files == nullptr)
            {
                files = new vector<FileInfo>();
            }

            do
            {
                // is file
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    files->push_back(FileInfo(&findData, toEnumerate));
                }
            } while (FindNextFile(findHandle, &findData));

            FindClose(findHandle);

            return true;
        }
        else
        {
            return false;
        }
    }

    std::vector<::Winnerino::Storage::FileInfo>* DirectoryEnumerator::GetFolders(winrt::hstring const& directory)
    {
        return GetFolders(directory, includeSpecials);
    }

    vector<FileInfo>* DirectoryEnumerator::GetFolders(winrt::hstring const& directory, bool const& includeNavigators)
    {
        CheckArguments(directory);

        vector<FileInfo>* folders = nullptr;

        hstring toEnumerate = directory.ends_with(L"\\") ? directory : directory + L"\\";

        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            const wchar_t dot[2]{ '.', '\0' };
            const wchar_t dotdot[3]{ '.', '.', '\0' };

            folders = new vector<FileInfo>();
            do
            {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    (includeNavigators || (wcscmp(findData.cFileName, dot) != 0 && wcscmp(findData.cFileName, dotdot) != 0)))
                {
                    folders->push_back(FileInfo(&findData, toEnumerate));
                }

            } while (FindNextFile(findHandle, &findData));

            FindClose(findHandle);
        }

        return folders;
    }

    vector<FileInfo>* DirectoryEnumerator::GetEntries(winrt::hstring const& path)
    {
        CheckArguments(path);

        vector<FileInfo>* entries = nullptr;

        hstring toEnumerate = path.ends_with(L"\\") ? path : path + L"\\";

        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            const wchar_t dot[2]{ '.', '\0' };
            const wchar_t dotdot[3]{ '.', '.', '\0' };

            entries = new vector<FileInfo>();
            do
            {
                if (wcscmp(findData.cFileName, dot) != 0 && wcscmp(findData.cFileName, dotdot) != 0)
                {
                    entries->push_back(FileInfo(&findData, toEnumerate));
                }

            } while (FindNextFile(findHandle, &findData));

            FindClose(findHandle);
        }

        return entries;
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