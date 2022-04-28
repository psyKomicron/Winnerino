#pragma once

#include "DriveSearchModel.g.h"

namespace winrt::Winnerino::implementation
{
    struct DriveSearchModel : DriveSearchModelT<DriveSearchModel>
    {
    private:
        winrt::hstring _driveName;
        bool _isSelected = true;
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

    public:
        DriveSearchModel(winrt::hstring driveName);

        bool IsSelected();
        void IsSelected(bool isSelected);
        winrt::hstring DriveName() { return _driveName; };

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value);
        void PropertyChanged(winrt::event_token const& token);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveSearchModel : DriveSearchModelT<DriveSearchModel, implementation::DriveSearchModel>
    {
    };
}
