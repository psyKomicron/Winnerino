#include "pch.h"
#include "DirectorySizeCalculator.h"
#include <array>
#include <atomic>
#include "fileapi.h"
#include "ppl.h"
#include "shlwapi.h"

constexpr int16_t BUFFER_MAX_SIZE = 1024;

using namespace std;
using namespace concurrency;
using namespace winrt::Windows::Foundation;

namespace winrt::Winnerino
{
    uint_fast64_t DirectorySizeCalculator::getSize(hstring const& path, bool parallelize)
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
                    uint_fast64_t fileSize = convertSize(findData.nFileSizeHigh, findData.nFileSizeLow);
                    dirSize += fileSize;
                    raiseProgress(fileSize);
                }
                else
                {
                    hstring filePath = hstring(findData.cFileName);
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
                    WCHAR combinedPath[BUFFER_MAX_SIZE];
                    PathCombine(combinedPath, wstr.c_str(), dir.c_str());
                    hstring deepPath = to_hstring(combinedPath) + L"\\";

                    uint_fast64_t size = getSize(deepPath, parallelize);
                    atomicDirSize.fetch_add(size);
                });
                dirSize += atomicDirSize.load();
            }
            else
            {
                size_t max = pathes.size();
                for (size_t i = 0; i < max; i++)
                {
                    WCHAR combinedPath[BUFFER_MAX_SIZE];
                    PathCombine(combinedPath, path.c_str(), pathes[i].c_str());
                    hstring deepPath = to_hstring(combinedPath) + L"\\";

                    uint_fast64_t size = getSize(deepPath, parallelize);
                    dirSize += size;
                }
            }
        }

        return dirSize;
    }

    inline uint_fast64_t DirectorySizeCalculator::convertSize(DWORD const& high, DWORD const& low)
    {
        return (static_cast<uint_fast64_t>(high) << 32) | low;
    }

    inline void DirectorySizeCalculator::raiseProgress(uint_fast64_t newSize)
    {
        m_event(nullptr, IReference{ newSize });
    }
}