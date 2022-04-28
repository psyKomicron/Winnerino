#include "pch.h"
#include "DrivesPage.xaml.h"
#if __has_include("DrivesPage.g.cpp")
#include "DrivesPage.g.cpp"
#include "fileapi.h"
#include "sysinfoapi.h"
#endif
using namespace winrt::Microsoft::UI::Xaml::Controls;

using namespace winrt;
using namespace std;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    DrivesPage::DrivesPage()
    {
        InitializeComponent();
    }

    void DrivesPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        SYSTEM_INFO systemInfo = SYSTEM_INFO();
        GetSystemInfo(&systemInfo);
        switch (systemInfo.wProcessorArchitecture)
        {
            case PROCESSOR_ARCHITECTURE_AMD64:
                CpuXTextBlock().Text(L"AMD/Intel 64");
                break;
            case PROCESSOR_ARCHITECTURE_ARM:
                CpuXTextBlock().Text(L"ARM");
                break;
            case PROCESSOR_ARCHITECTURE_ARM64:
                CpuXTextBlock().Text(L"ARM 64");
                break;
            case PROCESSOR_ARCHITECTURE_IA64:
                CpuXTextBlock().Text(L"Intel Itanium-based 64");
                break;
            case PROCESSOR_ARCHITECTURE_INTEL:
                CpuXTextBlock().Text(L"Intel");
                break;
            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                CpuXTextBlock().Text(L"Unknown");
                break;
        }
        CpuCountTextBlock().Text(to_hstring((uint32_t)systemInfo.dwNumberOfProcessors));
        CpuLevelTextBlock().Text(to_hstring((uint16_t)systemInfo.wProcessorLevel));
        CpuRevTextBlock().Text(to_hstring((uint16_t)systemInfo.wProcessorRevision));

        ListDrives();
    }

    void DrivesPage::ListDrives()
    {
        WCHAR drives[512]{};
        WCHAR* pointer = drives;
        GetLogicalDriveStrings(512, drives); // ignoring return value
        while (1)
        {
            if (*pointer == NULL)
            {
                break;
            }

            hstring driveName = hstring{ pointer };

            listView().Items().Append(Winnerino::DriveView{ driveName });
            listView().Items().Append(ListViewHeaderItem{});

            while (*pointer++);
        }
    }
}
