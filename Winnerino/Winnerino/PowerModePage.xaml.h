#pragma once

#include "PowerModePage.g.h"

namespace winrt::Winnerino::implementation
{
    struct PowerModePage : PowerModePageT<PowerModePage>
    {
    private:
        std::vector<winrt::Winnerino::PowerPlanViewModel2> viewModels{};
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> propertyChangedEvent;

        void listPowerPlans();
        void powerPlanItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    public:
        PowerModePage();

        bool EditPowerPlansButtonEnabled();

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        void refreshMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void listPlansMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void aboutMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void openWindowsSettingsMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct PowerModePage : PowerModePageT<PowerModePage, implementation::PowerModePage>
    {
    };
}
