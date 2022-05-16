#pragma once

#include "ExplorerPage.g.h"

namespace winrt::Winnerino::implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage>
    {
    private:
        winrt::event_token windowClosedToken;

    public:
        ExplorerPage();
        ~ExplorerPage();

        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Page_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TabView_TabCloseRequested(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args);
        void TabView_AddTabButtonClick(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        void AddTab(hstring const& header, hstring const& path);
        void mainWindow_Closed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&);
        void SavePage();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage, implementation::ExplorerPage>
    {
    };
}
