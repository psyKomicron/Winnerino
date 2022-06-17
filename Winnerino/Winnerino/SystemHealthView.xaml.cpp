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
#include "ppl.h"
#include "fileapi.h"
#include "SmartDevice.h"
#include "Helpers.h"
#include "DirectorySizeCalculator.h"
#include "DirectoryEnumerator.h"
#include <FileSearcher.h>

using namespace std;
using namespace ::Winnerino;
using namespace ::Winnerino::Devices;
using namespace ::Winnerino::Storage;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    SystemHealthView::SystemHealthView()
    {
        InitializeComponent();
        LoadDrivesInfos();
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
        /*SmartDevice smartDevice{ deviceName };
        auto&& info = smartDevice.GetInfo();
        Result().Text(info.Info() + L" | " + info.DeviceName());*/
    }

    IAsyncAction SystemHealthView::LoadDrivesInfos()
    {
        vector<hstring> drives{};
        map<hstring, uint64_t> driveSizes{};
        atomic_uint64_t atomicRecycleSize{};
        uint64_t totalSize{};
        wregex regExpr{ L"\\$recycle.bin", regex_constants::icase };

        DirectoryEnumerator enumerator{};
        list_local_drives(&drives);
        for (auto&& drive : drives)
        {
            Drives().Items().Append(DriveHealthView(drive));
        }

        co_return;
    }
}