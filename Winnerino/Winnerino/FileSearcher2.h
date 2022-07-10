#pragma once

#include <regex>
#include <concurrent_queue.h>
#include "FileInfo.h"
#include "DirectoryEnumerator.h"

namespace Winnerino::Storage
{
    class FileSearcher2
    {
    public:
        FileSearcher2() = default;
        FileSearcher2(winrt::hstring const& root);

        void Search(std::wregex query, std::vector<FileInfo>* results);

    private:
        winrt::hstring root;
        std::vector<std::thread> threads{};
        concurrency::concurrent_queue<winrt::hstring> pathQueue{};
        std::mutex queueMutex{};
        std::atomic_flag threadFlag{};

        //winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::hstring>> e_matchFound;

        void WorkerFunction(std::wregex query, std::vector<::Winnerino::Storage::FileInfo>* files);
        void GetFiles(std::wregex const& query, winrt::hstring path, std::vector<::Winnerino::Storage::FileInfo>* files);
        void QueuePathes(winrt::hstring const& root, std::wregex const& query, ::Winnerino::Storage::DirectoryEnumerator* enumerator, std::vector<::Winnerino::Storage::FileInfo>* validFiles);
    };
}

