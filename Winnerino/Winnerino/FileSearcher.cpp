#include "pch.h"
#include "FileSearcher.h"

#include <math.h>
#include "DirectoryEnumerator.h"

using namespace std;
using namespace winrt;

namespace Winnerino::Storage
{
    FileSearcher::FileSearcher()
    {
        threadFlag.test_and_set(memory_order_relaxed);
    }

    FileSearcher::FileSearcher(winrt::hstring const& root, uint16_t const& threads)
    {
        maxThreads = threads;
    }

    FileSearcher::~FileSearcher()
    {
        threadFlag.clear();
    }

    void FileSearcher::Search(wregex const& query, vector<hstring>* validFiles)
    {
        DirectoryEnumerator enumerator{};
        vector<hstring>* pathes = enumerator.EnumerateDrives();
        for (hstring drive : *pathes)
        {
            vector<hstring>* files = enumerator.EnumerateFiles(drive);
            for (size_t i = 0; i < files->size(); i++)
            {
                hstring file = files->operator[](i);
                if (regex_search(file.c_str(), query))
                {
                    validFiles->push_back(file);
                }
            }
            delete files;

            vector<hstring>* dirs = enumerator.EnumerateDirectories(drive);
            for (size_t i = 0; i < dirs->size(); i++)
            {
                unique_ptr<vector<hstring>> subDirs{ enumerator.EnumerateDirectories(dirs->at(i)) };
                if (subDirs)
                {
                    for (size_t j = 0; j < subDirs->size(); j++)
                    {
                        pathQueue.push(subDirs->at(j));
                    }
                }
            }
            delete dirs;
        }
        delete pathes;

        /*for (uint16_t i = 0; i < maxThreads; i++)
        {
            this->threads.push_back(thread{ &FileSearcher::WorkerFunction, this, query, &validFiles });
        }*/
        thread localThread = thread{ &FileSearcher::WorkerFunction, this, query, validFiles };
        thread localThread2 = thread{ &FileSearcher::WorkerFunction, this, query, validFiles };
        thread localThread3 = thread{ &FileSearcher::WorkerFunction, this, query, validFiles };
        localThread.join();
        localThread2.join();
        localThread3.join();
    }

    void FileSearcher::WorkerFunction(wregex query, vector<winrt::hstring>* files)
    {
        hstring path{};
        
        while (threadFlag.test() && !pathQueue.empty())
        {
            if (pathQueue.try_pop(path))
            {  
                auto&& id = this_thread::get_id();
                stringstream strStream{};
                strStream << id;

                OutputDebugString((to_hstring(strStream.str()) + L" : Working on \"" + path + L"\"\n").c_str());
                vector<hstring> temp{};
                GetFiles(query, path, &temp);

                // use block to destroy lock if i put more instructions below
                {
                    lock_guard<mutex> lock{ queueMutex };

                    size_t reserve = static_cast<size_t>(abs(static_cast<int>(files->size()) - static_cast<int>(temp.size())));
                    files->reserve(reserve);
                    for (size_t i = 0; i < temp.size(); i++)
                    {
                        files->push_back(std::move(temp.operator[](i)));
                    }
                }
            }
        }
    }

    void FileSearcher::GetFiles(wregex const& query, hstring path, vector<hstring>* files)
    {
        DirectoryEnumerator enumerator{};

        vector<hstring>* pathes = enumerator.EnumerateFiles(path);
        if (pathes)
        {
            for (size_t i = 0; i < pathes->size(); i++)
            {
                hstring filePath = std::move(pathes->at(i));
                if (regex_search(filePath.c_str(), query))
                {
                    //files->push_back(std::move(filePath));
                    files->push_back(filePath);
                    e_matchFound(Windows::Foundation::IInspectable{}, filePath);
                }
            }
            delete pathes;
        }
        
        pathes = enumerator.EnumerateDirectories(path);
        if (pathes)
        {
            for (size_t i = 0; i < pathes->size(); i++)
            {
                GetFiles(query, pathes->at(i), files);
            }
        }
    }

    void FileSearcher::QueuePathes(hstring const& root)
    {
    }
}
