#include "pch.h"
#include "DirectoryEnumerator.h"

#include "shlwapi.h"

using namespace winrt;
using namespace std;

namespace Winnerino::Storage
{
    vector<hstring>* DirectoryEnumerator::Enumerate(hstring const& toEnumerate)
    {
        hstring path;
        if (!toEnumerate.ends_with(L"\\"))
        {
            path = toEnumerate + L"\\";
        }

        vector<hstring>* pathes = nullptr;
        WIN32_FIND_DATA findData{};
        HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &findData);

        if (findHandle != INVALID_HANDLE_VALUE)
        {
            pathes = new vector<hstring>();
            do
            {
                hstring filePath{ findData.cFileName };
                if (filePath != L"." && filePath != L"..")
                {
                    pathes->push_back(path + filePath);
                }
            } while (FindNextFile(findHandle, &findData));
            FindClose(findHandle);
        }

        return pathes;
    }
}