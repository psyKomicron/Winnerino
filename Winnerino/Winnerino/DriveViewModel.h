#pragma once

#include "DriveViewModel.g.h"

namespace winrt::Winnerino::implementation
{
    struct DriveViewModel : DriveViewModelT<DriveViewModel>
    {
    public:
        DriveViewModel() = default;

        inline winrt::hstring Name() const;
        inline void Name(const winrt::hstring& value);
        inline winrt::hstring FriendlyName() const;
        inline void FriendlyName(const winrt::hstring& value);

        winrt::hstring ToString() const;

    private:
        winrt::hstring _name;
        winrt::hstring _friendlyName;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveViewModel : DriveViewModelT<DriveViewModel, implementation::DriveViewModel>
    {
    };
}
