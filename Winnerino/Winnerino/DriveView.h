#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "DriveView.g.h"

namespace winrt::Winnerino::implementation
{
    struct DriveView : DriveViewT<DriveView>
    {
    public:
        DriveView();

        inline winrt::hstring DriveName() const;
        inline void DriveName(const winrt::hstring& value);
        inline winrt::hstring FriendlyName() const;
        inline void FriendlyName(const winrt::hstring& value);

    private:
        /*winrt::hstring _name;
        winrt::hstring _friendlyName;*/

        static winrt::Microsoft::UI::Xaml::DependencyProperty _driveNameProperty;
        static winrt::Microsoft::UI::Xaml::DependencyProperty _friendlyNameProperty;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DriveView : DriveViewT<DriveView, implementation::DriveView>
    {
    };
}
