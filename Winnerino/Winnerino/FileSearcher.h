#pragma once
#include <regex>
#include <queue>
#include <concurrent_queue.h>

using namespace std;

namespace Winnerino::Storage
{
    class FileSearcher
    {
    public:
        FileSearcher();
        FileSearcher(winrt::hstring const& root, uint16_t const& threads = 3);
        ~FileSearcher();

        winrt::event_token MatchFound(winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::hstring> const& handler)
        {
            return e_matchFound.add(handler);
        };
        void MatchFound(winrt::event_token const& token)
        {
            e_matchFound.remove(token);
        };

        void Search(wregex const& query, vector<winrt::hstring>* results);

    private:
        int16_t maxThreads = 2;
        vector<thread> threads{};
        concurrency::concurrent_queue<winrt::hstring> pathQueue{};
        mutex queueMutex{};
        atomic_flag threadFlag{};
        
        winrt::event<winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::Foundation::IInspectable, winrt::hstring>> e_matchFound;

        void WorkerFunction(wregex query, vector<winrt::hstring>* files);
        void GetFiles(wregex const& query, winrt::hstring path, vector<winrt::hstring>* files);
        void QueuePathes(winrt::hstring const& root);
    };
}

