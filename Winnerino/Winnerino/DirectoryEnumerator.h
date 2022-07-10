#pragma once

#include "FileInfo.h"

namespace Winnerino::Storage
{
    class DirectoryEnumerator
    {
    public:
        DirectoryEnumerator() = default;
        DirectoryEnumerator(bool const& _includeSpecials) : includeSpecials{ _includeSpecials } {};
        ~DirectoryEnumerator() = default;

        std::vector<winrt::hstring>* Enumerate(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateDirectories(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateFiles(winrt::hstring const& path);
        std::vector<winrt::hstring>* EnumerateDrives();
        std::vector<::Winnerino::Storage::FileInfo>* GetFiles(winrt::hstring const& directory);

    private:
        bool includeSpecials = false;

        inline void CheckArguments(winrt::hstring const& path);
    };
}