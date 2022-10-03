#pragma once

#include <vector>
#include "FileInfo.h"
#include "DriveInfo.h"

namespace Winnerino::Storage
{
    class DirectoryEnumerator
    {
    public:
        DirectoryEnumerator() = default;
        DirectoryEnumerator(bool const& _includeSpecials) : includeSpecials{ _includeSpecials } {};
        ~DirectoryEnumerator() = default;

        std::vector<winrt::hstring>* Enumerate(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateFolders(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateFiles(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateDrives();
        std::vector<::Winnerino::Storage::DriveInfo>* GetDrives();
        std::vector<::Winnerino::Storage::FileInfo>* GetFiles(winrt::hstring const& directory);
        bool GetFiles(winrt::hstring const& directory, std::vector<::Winnerino::Storage::FileInfo>* files);
        std::vector<::Winnerino::Storage::FileInfo>* GetFolders(winrt::hstring const& directory);
        std::vector<::Winnerino::Storage::FileInfo>* GetFolders(winrt::hstring const& directory, bool const& includeNavigators);
        std::vector<::Winnerino::Storage::FileInfo>* GetEntries(winrt::hstring const& path);

    private:
        bool includeSpecials = false;

        inline void CheckArguments(winrt::hstring const& path);
    };
}