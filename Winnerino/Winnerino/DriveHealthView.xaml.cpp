#include "pch.h"
#include "DriveHealthView.xaml.h"
#if __has_include("DriveHealthView.g.cpp")
#include "DriveHealthView.g.cpp"
#endif

#include <array>
#include <vector>
#include <atomic>
#include <regex>
#include <ppl.h>
#include <pplawait.h>
#include <fileapi.h>
#include <shellapi.h>
#include "SmartDevice.h"
#include "DirectorySizeCalculator.h"
#include "DirectoryEnumerator.h"
#include "Helper.h"
#include "FileSearcher2.h"

using namespace std;

using namespace ::Winnerino;
using namespace ::Winnerino::Storage;

using namespace winrt;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;

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
        mainDrive = (path == L"c:\\" || path == L"C:\\");
        if (!mainDrive)
        {
            Expanders().Items().RemoveAt(2);
        }
        drivePath = hstring(path);
    }

    void DriveHealthView::ClearRecycleBin_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        HWND windowHwnd = GetWindowFromWindowId(MainWindow::Current().Id());
        HRESULT retCode = SHEmptyRecycleBin(windowHwnd, drivePath.c_str(), SHERB_NOSOUND | SHERB_NOCONFIRMATION);
        if (retCode != S_OK)
        {
            // I18N: Failed to clear recycle bin
            MainWindow::Current().NotifyError(retCode, L"Failed to clear recycle bin");
        }

        LoadInfos();
    }

    IAsyncAction DriveHealthView::OpenBinExplorer_Click(IInspectable const&, RoutedEventArgs const&)
    {
        co_await winrt::Windows::System::Launcher::LaunchFolderPathAsync(drivePath + L"$recycle.bin");
    }

    IAsyncAction DriveHealthView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        if (firstLoad)
        {
            firstLoad = false;
            LoadInfos();
        }
        co_return;
    }


    void DriveHealthView::LoadInfos()
    {
        LoadingProgressRing().IsIndeterminate(true);
        DriveStatusEllipse().Fill(Application::Current().Resources().Lookup(box_value(L"SystemFillColorCautionBrush")).as<winrt::Microsoft::UI::Xaml::Media::Brush>());

        constexpr uint16_t decimals = 1;
        uint64_t totalSize{};
        DirectoryEnumerator enumerator{};
        DirectorySizeCalculator calculator{};

        WCHAR driveName[MAX_PATH + 1]{};
        DriveName(!GetVolumeInformation(drivePath.c_str(), driveName, MAX_PATH + 1, NULL, NULL, NULL, NULL, 0) ? to_hstring(driveName) : drivePath);

        ULARGE_INTEGER totalNumbersOfFreeBytes, totalNumberOfBytes;
        if (GetDiskFreeSpaceEx(drivePath.c_str(), NULL, &totalNumberOfBytes, &totalNumbersOfFreeBytes))
        {
            uint64_t driveFreeSpace = convert_large_uint(totalNumbersOfFreeBytes);
            totalSize = convert_large_uint(totalNumberOfBytes);

            hstring totalFormattedSize = format_size(static_cast<double>(totalSize), decimals);

            DriveFreeSpace(driveFreeSpace);
            DriveTotalSpace(totalSize);

            DispatcherQueue().TryEnqueue([this, ctotalSize = totalSize, cdriveFreeSpace = driveFreeSpace]()
            {
                DriveProgressBar().Value(ctotalSize - cdriveFreeSpace);
                // I18N: "used" & "free"
                Subheader1().Text(format_size(static_cast<double>(ctotalSize - cdriveFreeSpace), decimals) + L" used");
                Subheader2().Text(format_size(static_cast<double>(cdriveFreeSpace), decimals) + L" free");
            });

            uint64_t recycleSize{};
            SHQUERYRBINFO binInfo{ sizeof(SHQUERYRBINFO) };
            if (SHQueryRecycleBin(drivePath.c_str(), &binInfo) == S_OK)
            {
                DispatcherQueue().TryEnqueue([this, i64NumItems = binInfo.i64NumItems, i64Size = binInfo.i64Size, ctotalFormattedSize = totalFormattedSize]()
                {
                    RecycleBinNumberOfItems().Text(to_hstring(i64NumItems));
                    RecycleBinTotalSize().Value(i64Size);
                    RecycleBinRatio().Text(format_size(static_cast<double>(i64Size), decimals) + L"/" + ctotalFormattedSize);
                });
            }

            if (mainDrive)
            {
                WCHAR tempPath[MAX_PATH]{};
                if (GetTempPath2(MAX_PATH, tempPath))
                {
                    concurrency::create_task([this, path = to_hstring(tempPath)]()
                    {
                        DirectorySizeCalculator calculator{};
                        uint64_t tempSize = calculator.GetSize(path);
                        DispatcherQueue().TryEnqueue([this, ctempSize = tempSize]()
                        {
                            TempDataTotalSize().Value(ctempSize);
                        });
                    });
                }
            }

            // TODO: Support cancellation on app close/user action
            concurrency::create_task([this, ctotalFormattedSize = totalFormattedSize]()
            {
                wregex sysFiles{ L"(DumpStack\\.log)|(desktop\\.ini)|(hiberfil\\.sys)|(pagefile\\.sys)|(swapfile\\.sys)", regex_constants::icase };
                FileSearcher2 searcher = FileSearcher2(drivePath);
                std::vector<FileInfo>* validFiles = new std::vector<FileInfo>();
                searcher.Search(sysFiles, validFiles);

                DispatcherQueue().TryEnqueue([this, cctotalFormattedSize = ctotalFormattedSize, validFiles]()
                {
                    uint64_t sysFilesTotalSize = 0;
                    for (size_t i = 0; i < validFiles->size(); i++)
                    {
                        sysFilesTotalSize += validFiles->at(i).Size();

                        TextBlock textBlock{};
                        textBlock.Text(validFiles->at(i).Path());
                        SystemFilesListView().Items().Append(textBlock);
                    }
                    delete validFiles;

                    SystemFilesRatio().Text(format_size(static_cast<double>(sysFilesTotalSize), decimals) + L"/" + cctotalFormattedSize);
                    SystemFilesTotalSize().Value(static_cast<double>(sysFilesTotalSize));
                    LoadingProgressRing().IsIndeterminate(false);
                    DriveStatusEllipse().Fill(Application::Current().Resources().Lookup(box_value(L"SystemFillColorSuccessBrush")).as<winrt::Microsoft::UI::Xaml::Media::Brush>());
                });
            });
        }
        else
        {
            DriveStatusEllipse().Fill(Application::Current().Resources().Lookup(box_value(L"SystemFillColorCriticalBrush")).as<winrt::Microsoft::UI::Xaml::Media::Brush>());
            LoadingProgressRing().IsIndeterminate(false);
        }
    }
}