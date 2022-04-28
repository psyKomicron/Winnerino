#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "DriveView.g.h"

namespace winrt::Winnerino::implementation
{
    struct DriveView : DriveViewT<DriveView>
    {
    private:
        winrt::hstring _driveName;
        int64_t _driveFreeBytes;
        int64_t _driveSize;
        winrt::hstring _fileSystemType;
        int64_t _maxFileNameLength;
        int64_t _volumeSerialNumber;
        winrt::hstring _driveType;

    public:
        DriveView();
        DriveView(winrt::hstring const& driveName);

        winrt::hstring DriveName() { return _driveName; };
        winrt::hstring DriveFreeSpace() { return winrt::to_hstring(_driveFreeBytes); };
        winrt::hstring DriveTotalSpace() { return winrt::to_hstring(_driveSize); };
        winrt::hstring FileSystemType() { return _fileSystemType; };
        winrt::hstring VolumeSerialNumber() { return to_hstring(_volumeSerialNumber); };
        winrt::hstring MaxFileLength() { return winrt::to_hstring(_maxFileNameLength); };
        winrt::hstring DriveType() { return _driveType; };
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        bool LoadForNVME(HANDLE const& diskHandle);
        bool LoadForATA(HANDLE const& diskHandle);
        inline hstring ConvertToHString(UCHAR* const& toConvert, size_t const& length);
        inline Microsoft::UI::Xaml::Controls::TextBlock CreateTextBlock(hstring const& text);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveView : DriveViewT<DriveView, implementation::DriveView>
    {
    };
}
