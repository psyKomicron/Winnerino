#include "pch.h"
#include "DriveHealthView.xaml.h"
#if __has_include("DriveHealthView.g.cpp")
#include "DriveHealthView.g.cpp"
#endif

#include <array>
#include <vector>
#include <atomic>
#include <regex>
#include "ppl.h"
#include "fileapi.h"
#include "SmartDevice.h"
#include "DirectorySizeCalculator.h"
#include "DirectoryEnumerator.h"
#include "Helpers.h"

using namespace std;
using namespace ::Winnerino;
using namespace ::Winnerino::Storage;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    DriveHealthView::DriveHealthView()
    {
        InitializeComponent();
    }

    DriveHealthView::DriveHealthView(hstring const& path) : DriveHealthView()
    {
        LoadInfos(path);
    }

    void DriveHealthView::LoadInfos(hstring const& path)
    {
        const uint16_t decimals = 1;
        uint64_t recycleSize{};
        uint64_t totalSize{};
        wregex regExpr{ L"\\$recycle.bin", regex_constants::icase };
        DirectoryEnumerator enumerator{};
        DirectorySizeCalculator calculator{};
        ULARGE_INTEGER totalNumbersOfFreeBytes, totalNumberOfBytes;

        DriveName(path);
        if (GetDiskFreeSpaceEx(path.c_str(), NULL, &totalNumberOfBytes, &totalNumbersOfFreeBytes))
        {
            uint64_t driveFreeSpace = convert_large_uint(totalNumbersOfFreeBytes);
            totalSize = convert_large_uint(totalNumberOfBytes);

            DriveFreeSpace(driveFreeSpace);
            DriveTotalSpace(totalSize);
            DriveProgressBar().Value(totalSize - driveFreeSpace);

            // I18N
            double formattedSize = static_cast<double>(totalSize - driveFreeSpace);
            hstring ext = format_size(&formattedSize, decimals);
            Subheader1().Text(to_hstring(formattedSize) + ext + L" used");

            formattedSize = static_cast<double>(driveFreeSpace);
            ext = format_size(&formattedSize, decimals);
            Subheader2().Text(to_hstring(formattedSize) + ext + L" free");
        }
        else
        {
            MainWindow::Current().NotifyError(GetLastError(), L"Failed to get volume infos (" + path + L")");
        }

        vector<hstring>* paths = enumerator.Enumerate(path);
        for (size_t j = 0; j < paths->size(); j++)
        {
            if (regex_search(paths->operator[](j).c_str(), regExpr))
            {
                uint64_t size = calculator.getSize(paths->operator[](j) + L"\\");
                recycleSize += size;
                break;
            }
        }
        delete paths;


        double totalFormattedSize = static_cast<double>(totalSize);
        hstring totalFormattedSizeExt = format_size(&totalFormattedSize, decimals);

        double formattedSize = static_cast<double>(recycleSize);
        hstring ext1 = format_size(&formattedSize, decimals);

        RecycleBinTotalSize().Value(recycleSize);
        RecycleBinRatio().Text(to_hstring(formattedSize) + ext1 + L"/" + to_hstring(totalFormattedSize) + totalFormattedSizeExt);


        if (path == L"C:\\")
        {
            WCHAR tempPath[MAX_PATH]{};
            if (GetTempPath2(MAX_PATH, tempPath))
            {
                uint64_t tempSize = calculator.getSize(to_hstring(tempPath));
                TempDataTotalSize().Value(tempSize);
            }
        }
        else
        {
            Expanders().Items().RemoveAt(2);
        }


        // get sys files
        auto&& pathes = enumerator.Enumerate(path);
        wregex sysFiles{ L"(.\\.sys$)|(.\\.log)|(desktop\\.ini)", regex_constants::icase };
        

    }

    void DriveHealthView::GetSystemFiles(hstring const& path)
    {
    }
}
