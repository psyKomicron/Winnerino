#include "pch.h"
#include "fileapi.h"
#include "ioapiset.h"
#include "DriveView.xaml.h"
#if __has_include("DriveView.g.cpp")
#include "DriveView.g.cpp"
#endif
using namespace winrt::Microsoft::UI::Xaml::Controls;

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    DriveView::DriveView()
    {
        _driveName = L"";
        _driveFreeBytes = 0;
        _driveSize = 0;
        _fileSystemType = L"";
        _maxFileNameLength = 0;
        _volumeSerialNumber = 0;
        _driveType = L"";
        InitializeComponent();
    }

    DriveView::DriveView(winrt::hstring const& driveName)
    {
        _driveName = driveName;

        WCHAR friendlyName[MAX_PATH + 1];
        DWORD volumeSerialNumber = 0;
        DWORD volumeMaxComponentLength = 0;
        DWORD systemFlags = 0;
        WCHAR fileSystemName[MAX_PATH];
        if (GetVolumeInformation(driveName.c_str(), friendlyName, MAX_PATH + 1, &volumeSerialNumber, &volumeMaxComponentLength, &systemFlags, fileSystemName, MAX_PATH))
        {
            _volumeSerialNumber = volumeSerialNumber;
            _maxFileNameLength = volumeMaxComponentLength;
            _fileSystemType = fileSystemName;

            switch (GetDriveType(driveName.c_str()))
            {
                case DRIVE_REMOVABLE:
                    _driveType = L"Removable drive";
                    break;
                case DRIVE_FIXED:
                    _driveType = L"Hard drive";
                    break;
                case DRIVE_REMOTE:
                    _driveType = L"Remote drive";
                    break;
                case DRIVE_CDROM:
                    _driveType = L"CD";
                    break;
                case DRIVE_RAMDISK:
                    _driveType = L"Ram disk";
                    break;
                default:
                    _driveType = L"Unknow drive type";
                    break;
            }

            // no initialization since it will be done by GetDiskFreeSpace
            ULARGE_INTEGER totalNumbersOfFreeBytes, totalNumberOfBytes;
            if (GetDiskFreeSpaceEx(driveName.c_str(), NULL, &totalNumberOfBytes, &totalNumbersOfFreeBytes))
            {
                _driveFreeBytes = (static_cast<int64_t>(totalNumbersOfFreeBytes.HighPart) << 32) | totalNumbersOfFreeBytes.LowPart;
                _driveSize = (static_cast<int64_t>(totalNumberOfBytes.HighPart) << 32) | totalNumberOfBytes.LowPart;
            }
            InitializeComponent();
        }
        else
        {
            InitializeComponent();
            MainWindow::Current().NotifyError(GetLastError());
        }
    }

    void DriveView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        double ratio = (static_cast<double>(_driveSize - _driveFreeBytes) / _driveSize);
        double width = (ActualWidth() - Padding().Right) * ratio;

#if TRUE
        std::string fileName = to_string(_driveName);
        fileName = "\\\\.\\" + fileName.substr(0, fileName.size() - 1);

        // only works with elevated privileges
        HANDLE diskHandle = CreateFile(to_hstring(fileName).c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (diskHandle != INVALID_HANDLE_VALUE)
        {
            GET_LENGTH_INFORMATION lengthInfo{ 0 };
            if (DeviceIoControl(diskHandle,
                IOCTL_DISK_GET_LENGTH_INFO,
                NULL,
                0,
                &lengthInfo,
                sizeof(lengthInfo),
                NULL,
                NULL))
            {
                _driveSize = (static_cast<int64_t>(lengthInfo.Length.HighPart) << 32) | lengthInfo.Length.LowPart;
                width = ActualWidth() * ((static_cast<double>(_driveSize) - _driveFreeBytes) / _driveSize);

                MainWindow::Current().NotifyUser(L"Successfully retreived disk info for " + _driveName, InfoBarSeverity::Success);
            }
            else
            {
                DWORD lastError = GetLastError();
                if (lastError == ERROR_ACCESS_DENIED)
                {
                    MainWindow::Current().NotifyError(lastError, L"Failed to get " + _driveName + L" drive information. Try to run with administator privileges");
                }
                else
                {
                    MainWindow::Current().NotifyError(lastError, L"Failed to get drive handle");
                }
            }

            CloseHandle(diskHandle);
        }
        else
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_ACCESS_DENIED)
            {
                MainWindow::Current().NotifyError(lastError, L"Failed to get " + _driveName + L" drive information. Try to run with administator privileges");
            }
            else
            {
                MainWindow::Current().NotifyError(lastError, L"Failed to get drive handle");
            }
        }
#endif // FALSE

        ratioDisplay().Width(width);
    }
}
