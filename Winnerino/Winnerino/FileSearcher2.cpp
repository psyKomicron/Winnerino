#include "pch.h"
#include "FileSearcher2.h"

using namespace std;
using namespace winrt;

namespace Winnerino::Storage
{
    FileSearcher2::FileSearcher2(hstring const& root)
    {
        this->root = root;
    }

    void FileSearcher2::Search(wregex query, vector<FileInfo>* results)
    {
        DirectoryEnumerator enumerator{};

        if (root.empty())
        {
            unique_ptr<vector<hstring>> pathes{ enumerator.EnumerateDrives() };
            for (hstring drive : *pathes)
            {
                QueuePathes(drive, query, &enumerator, results);
            }
        }
        else
        {
            QueuePathes(root, query, &enumerator, results);
        }

        threadFlag.test_and_set(memory_order_relaxed);
        thread localThread = thread{ &FileSearcher2::WorkerFunction, this, query, results };
        thread localThread2 = thread{ &FileSearcher2::WorkerFunction, this, query, results };
        thread localThread3 = thread{ &FileSearcher2::WorkerFunction, this, query, results };
        thread localThread4 = thread{ &FileSearcher2::WorkerFunction, this, query, results };
        localThread.join();
        localThread2.join();
        localThread3.join();
        localThread4.join();
    }


    void FileSearcher2::WorkerFunction(wregex query, vector<FileInfo>* files)
    {
        hstring path{};

        while (threadFlag.test() && !pathQueue.empty())
        {
            if (pathQueue.try_pop(path))
            {
#ifdef _DEBUG
                auto&& id = this_thread::get_id();
                stringstream strStream{ "" };
                strStream << "0x" << hex << id;

                OutputDebugString((to_hstring(strStream.str()) + L" : Working on \"" + path + L"\"\n").c_str());
#endif // _DEBUG

                vector<FileInfo> temp{};
                try
                {
                    GetFiles(query, path, &temp);
                }
                catch (hresult_error){}
                catch (std::exception){}

                if (temp.size() > 0)
                {
                    lock_guard<mutex> lock{ queueMutex };

                    size_t reserve = static_cast<size_t>(files->size() + temp.size());
                    files->reserve(reserve);
                    for (size_t i = 0; i < temp.size(); i++)
                    {
                        files->push_back(std::move(temp.at(i)));
                    }
                }
            }
        }
    }

    void FileSearcher2::GetFiles(wregex const& query, hstring path, vector<FileInfo>* files)
    {
        unique_ptr<vector<hstring>> pathes{ new vector<hstring>() };

        hstring toEnumerate = path.ends_with(L"\\") ? path : path + L"\\";
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // is file
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    FileInfo fileInfo = FileInfo(&findData, path);
                    if (regex_search(fileInfo.Name().c_str(), query))
                    {
                        files->push_back(std::move(fileInfo));
                        //e_matchFound(Windows::Foundation::IInspectable{}, filePath);
                    }
                }
                // is directory
                else
                {
                    hstring dirName = to_hstring(findData.cFileName);
                    if (dirName != L"." && dirName != L"..") // can be optimised
                    {
                        if (regex_search(dirName.c_str(), query))
                        {
                            files->push_back(FileInfo(&findData, path));
                        }

                        pathes->push_back(toEnumerate + dirName);
                    }
                }
            } 
            while (FindNextFile(findHandle, &findData) && threadFlag.test());

            FindClose(findHandle);
        }

        for (size_t i = 0; i < pathes->size(); i++)
        {
            GetFiles(query, pathes->at(i), files);
        }
    }

    void FileSearcher2::QueuePathes(hstring const& rootPath, wregex const& query, DirectoryEnumerator* enumerator, vector<FileInfo>* validFiles)
    {
        unique_ptr<vector<hstring>> dirs{ new vector<hstring>() };
        hstring toEnumerate = rootPath.ends_with(L"\\") ? rootPath : rootPath + L"\\";
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((toEnumerate + L"*").c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // is file
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    FileInfo fileInfo = FileInfo(&findData, rootPath);
                    if (regex_search(fileInfo.Name().data(), query))
                    {
                        validFiles->push_back(std::move(fileInfo));
                        //e_matchFound(Windows::Foundation::IInspectable{}, filePath);
                    }
                }
                // is directory
                else
                {
                    hstring dirName = to_hstring(findData.cFileName);
                    if (dirName != L"." && dirName != L"..") // can be optimised
                    {
                        dirs->push_back(toEnumerate + dirName);
                    }
                }
            } while (FindNextFile(findHandle, &findData));

            FindClose(findHandle);
        }
        
        if (dirs->size() > 0u)
        {
            for (size_t i = 0; i < dirs->size(); i++)
            {
                hstring subPath = std::move(dirs->at(i));
                unique_ptr<vector<hstring>> subDirs{ enumerator->EnumerateFolders(subPath) };
                if (subDirs)
                {
                    for (size_t j = 0; j < subDirs->size(); j++)
                    {
                        pathQueue.push(subDirs->at(j));
                    }
                }

                unique_ptr<vector<FileInfo>> subFiles{ enumerator->GetFiles(subPath) };
                if (subFiles)
                {
                    for (size_t j = 0; j < subFiles->size(); j++)
                    {
                        if (regex_search(subFiles->at(j).Name().c_str(), query))
                        {
                            validFiles->push_back(subFiles->at(j));
                        }
                    }
                }
            }
        }
    }
}