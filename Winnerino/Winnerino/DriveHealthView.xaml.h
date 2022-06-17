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

    private:
        double _driveFreeSpace;
        double _driveTotalSpace;
        winrt::hstring _driveName;
        double _percentage;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        void DriveName(hstring const& value)
        {
            _driveName = value;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveName" });
        };
        void DriveFreeSpace(double const& value)
        {
            _driveFreeSpace = value;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveFreeSpace" });
        };
        void DriveTotalSpace(double const& value)
        {
            _driveTotalSpace = value;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"DriveTotalSpace" });
        };
        void Percentage(double value)
        {
            _percentage = value;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"Percentage" });
        };
        void LoadInfos(hstring const& path);
        void GetSystemFiles(hstring const& path);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveHealthView : DriveHealthViewT<DriveHealthView, implementation::DriveHealthView>
    {
    };
}
