#include "pch.h"
#include "DirectorySizeCalculator.h"

#include <vector>
#include <array>
#include <atomic>
#include <fileapi.h>
#include <ppl.h>
#include <Shlwapi.h>
#include "Helpers.h"

using namespace std;
using namespace concurrency;
using namespace winrt::Windows::Foundation;

namespace Winnerino::Storage
{
    uint_fast64_t DirectorySizeCalculator::GetSize(hstring const& path, bool parallelize)
    {
        if (path.starts_with(L"\\")) // Prevents infinite looping when the directory name is invalid (often buffer too small)
        {
            return 0;
        }

        uint_fast64_t dirSize = 0;
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            vector<hstring> pathes = vector<hstring>();
            do
            {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    uint_fast64_t fileSize = convert_large_uint(findData.nFileSizeHigh, findData.nFileSizeLow);
                    dirSize += fileSize;
                    RaiseProgress(fileSize);
                }
                else
                {
                    hstring filePath = hstring(std::move(findData.cFileName));
                    if (filePath != L"." && filePath != L"..")
                    {
                        pathes.push_back(filePath);
                    }
                }
            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
            
            atomic_uint_fast64_t atomicDirSize = atomic_uint_fast64_t();
            if (parallelize && pathes.size() > 1)
            {
                parallel_for_each(begin(pathes), end(pathes), [this, wstr = path, &atomicDirSize, &parallelize](const hstring& dir)
                {
                    WCHAR combinedPath[ALTERNATE_MAX_PATH];
                    PathCombine(combinedPath, wstr.c_str(), dir.c_str());
                    hstring deepPath = to_hstring(combinedPath) + L"\\";

                    uint_fast64_t size = GetSize(deepPath, parallelize);
                    atomicDirSize.fetch_add(size);
                });
                dirSize += atomicDirSize.load();
            }
            else
            {
                size_t max = pathes.size();
                for (size_t i = 0; i < max; i++)
                {
                    WCHAR combinedPath[ALTERNATE_MAX_PATH];
                    PathCombine(combinedPath, path.c_str(), pathes[i].c_str());
                    hstring deepPath = to_hstring(combinedPath) + L"\\";

                    uint_fast64_t size = GetSize(deepPath, parallelize);
                    dirSize += size;
                }
            }
        }

        return dirSize;
    }

    inline void DirectorySizeCalculator::RaiseProgress(uint_fast64_t newSize)
    {
        m_event(nullptr, IReference{ newSize });
    }
}