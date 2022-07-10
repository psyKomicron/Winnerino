#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "DriveHealthView.g.h"

namespace winrt::Winnerino::implementation
{
    struct DriveHealthView : DriveHealthViewT<DriveHealthView>
    {
        DriveHealthView();
        DriveHealthView(hstring const& path);

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        winrt::hstring DriveName()
        {
            return _driveName;
        };
        double DriveFreeSpace()
        {
            return _driveFreeSpace;
        };
        double DriveTotalSpace()
        {
            return _driveTotalSpace;
        };

        void ClearRecycleBin_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction OpenBinExplorer_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        bool mainDrive = false;
        bool firstLoad = true;
        double _driveFreeSpace = 0;
        double _driveTotalSpace = 0;
        winrt::hstring _driveName;
        winrt::hstring drivePath;
        double _percentage = 0.;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        void DriveName(hstring const& value)
        {
            _driveName = value;
            if (DispatcherQueue().HasThreadAccess())
            {
                e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveName" });
            }
            else
            {
                DispatcherQueue().TryEnqueue([this]()
                {
                    e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveName" });
                });
            }
        };
        void DriveFreeSpace(double const& value)
        {
            _driveFreeSpace = value;
            if (DispatcherQueue().HasThreadAccess())
            {
                e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveFreeSpace" });
            }
            else
            {
                DispatcherQueue().TryEnqueue([this]()
                {
                    e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveFreeSpace" });
                });
            }
        };
        void DriveTotalSpace(double const& value)
        {
            _driveTotalSpace = value;
            if (DispatcherQueue().HasThreadAccess())
            {
                e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveTotalSpace" });
            }
            else
            {
                DispatcherQueue().TryEnqueue([this]()
                {
                    e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveTotalSpace" });
                });
            }
        };
        void Percentage(double value)
        {
            _percentage = value;
            if (DispatcherQueue().HasThreadAccess())
            {
                e_propertyChanged(*this, PropertyChangedEventArgs{ L"Percentage" });
            }
            else
            {
                DispatcherQueue().TryEnqueue([this]()
                {
                    e_propertyChanged(*this, PropertyChangedEventArgs{ L"Percentage" });
                });
            }
        };
        void LoadInfos();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveHealthView : DriveHealthViewT<DriveHealthView, implementation::DriveHealthView>
    {
    };
}
