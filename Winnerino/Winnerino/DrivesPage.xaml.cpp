#include "pch.h"
#include "fileapi.h"
#include "DrivesPage.xaml.h"
#if __has_include("DrivesPage.g.cpp")
#include "DrivesPage.g.cpp"
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
        ListDrives();
    }

    void DrivesPage::ListDrives()
    {
#if FALSE
        WCHAR driveName[MAX_PATH]{ 0 };
        HANDLE findHandle = FindFirstVolume(driveName, MAX_PATH);
        do
        {
            hstring driveGUID = to_hstring(driveName);
            vector<hstring> driveVolumes = vector<hstring>();

            WCHAR volumes[MAX_PATH]{ 0 };
            LPWSTR pointer = volumes;
            DWORD outputLength = 0;
            if (GetVolumePathNamesForVolumeName(driveName, volumes, MAX_PATH, &outputLength))
            {
                uint16_t maxIterations = 0;
                while (maxIterations < outputLength)
                {
                    if (*pointer == NULL)
                    {
                        break;
                    }
                    else
                    {
                        driveVolumes.push_back(to_hstring(pointer));
                    }
                    maxIterations++;
                    while (*pointer++);
                }
            }

            if (GetVolumeInformation(driveVolumes[0].c_str(), ))
        } while (FindNextVolume(findHandle, driveName, MAX_PATH));
        FindVolumeClose(findHandle);
#endif // 0


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
