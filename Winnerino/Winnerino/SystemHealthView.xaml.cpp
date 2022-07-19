#include "pch.h"
#include "SystemHealthView.xaml.h"
#if __has_include("SystemHealthView.g.cpp")
#include "SystemHealthView.g.cpp"
#endif

#include <array>
#include <vector>
#include <map>
#include <atomic>
#include <regex>
#include <ppl.h>
#include <fileapi.h>
#include <shellapi.h>
#include <math.h>
#include "SmartDevice.h"
#include "Helper.h"
#include "DirectorySizeCalculator.h"
#include "DirectoryEnumerator.h"
#include <FileSearcher.h>

using namespace std;
using namespace ::Winnerino;
using namespace ::Winnerino::Devices;
using namespace ::Winnerino::Storage;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    SystemHealthView::SystemHealthView()
    {
        InitializeComponent();
        LoadDrivesInfos();
    }

    Brush SystemHealthView::TotalUsedForeground()
    {
        if (_totalUsedSpace < 75.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorSuccessBrush")).as<Brush>();
        }
        else if (_totalUsedSpace < 85.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorNeutralBrush")).as<Brush>();
        }
        else if (_totalUsedSpace < 90.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorCautionBrush")).as<Brush>();
        }
        else
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorCriticalBrush")).as<Brush>();
        }
    }

    Brush SystemHealthView::SystemGeneralForeground()
    {
        if (_systemGeneralHealth > 75.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorSuccessBrush")).as<Brush>();
        }
        else if (_systemGeneralHealth > 50.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorNeutralBrush")).as<Brush>();
        }
        else if (_systemGeneralHealth > 30.)
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorCautionBrush")).as<Brush>();
        }
        else
        {
            return Application::Current().Resources().Lookup(box_value(L"SystemFillColorCriticalBrush")).as<Brush>();
        }
    }

    void SystemHealthView::DeviceNameBox_QuerySubmitted(AutoSuggestBox const& sender, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        if (!args.QueryText().empty())
        {
            GetSmartInfos(args.QueryText());
        }
    }

    void SystemHealthView::GetSmartInfos(winrt::hstring const&)
    {
        
    }

    IAsyncAction SystemHealthView::LoadDrivesInfos()
    {
        auto&& lifetime = get_strong();

        vector<hstring> drives{};
        uint64_t totalRecycleSize = 0, totalFreeSpace = 0, totalSpace = 0;
        SHQUERYRBINFO queryBinInfo{ sizeof(SHQUERYRBINFO) };
        ULARGE_INTEGER totalNumbersOfFreeBytes{}, totalNumberOfBytes{};

        list_local_drives(&drives);
        for (auto&& drive : drives)
        {
            Drives().Items().Append(DriveHealthView(drive));

            SHQueryRecycleBin(drive.c_str(), &queryBinInfo);
            totalRecycleSize += queryBinInfo.i64Size;

            if (GetDiskFreeSpaceEx(drive.c_str(), NULL, &totalNumberOfBytes, &totalNumbersOfFreeBytes))
            {
                totalFreeSpace += totalNumbersOfFreeBytes.QuadPart;
                totalSpace += totalNumberOfBytes.QuadPart;
            }
        }

        double totalSizeD = static_cast<double>(totalSpace);
        TotalUsedSpace(round(((totalSpace - totalFreeSpace) / totalSizeD) * 100));
        hstring totalSizeFormatted = format_size(totalSizeD, 1);
        hstring totalFreeSizeFormatted = format_size(static_cast<double>(totalFreeSpace), 1);
        TotalUsedSize().Text(totalFreeSizeFormatted + L"/" + totalSizeFormatted);

        uint8_t score = 4;
        double ratio = totalRecycleSize / totalSizeD;
        if (ratio > 0.3)
        {
            score--;
        }
        ratio = totalFreeSpace / totalSizeD;
        if (ratio > 0.8)
        {
            score--;
        }

        WCHAR tempPath[MAX_PATH]{};
        if (GetTempPath2(MAX_PATH, tempPath))
        {
            DirectorySizeCalculator calculator{};
            uint64_t tempSize = calculator.GetSize(to_hstring(tempPath));
            ratio = tempSize / totalSizeD;
            if (ratio > 0.2)
            {
                score--;
            }
        }
        
        {
            User user = User::GetDefault();
            StorageFolder downloadsFolder = co_await KnownFolders::GetFolderForUserAsync(user, KnownFolderId::DownloadsFolder);
            DirectorySizeCalculator calculator{};
            uint64_t downloadsFolderSize = calculator.GetSize(downloadsFolder.Path() + L"\\");
            if ((downloadsFolderSize / totalSizeD) > 0.2)
            {
                score--;
            }
        }

        lifetime->DispatcherQueue().TryEnqueue([lifetime, cscore = score]()
        {
            lifetime->SystemGeneralHealth(round((cscore / 4.0) * 100));
        });
    }
}