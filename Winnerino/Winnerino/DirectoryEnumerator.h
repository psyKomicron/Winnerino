#pragma once

namespace Winnerino::Storage
{
    class DirectoryEnumerator
    {
    public:
        DirectoryEnumerator() = default;
        ~DirectoryEnumerator() = default;

        std::vector<winrt::hstring>* Enumerate(winrt::hstring const& path);
    };
}