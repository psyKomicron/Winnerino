#pragma once

#include "DrivesPage.g.h"

namespace winrt::Winnerino::implementation
{
    struct DrivesPage : DrivesPageT<DrivesPage>
    {
    public:
        DrivesPage();
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    /// <summary>
    /// Private functions
    /// </summary>
    private:
        void ListDrives();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct DrivesPage : DrivesPageT<DrivesPage, implementation::DrivesPage>
    {
    };
}
