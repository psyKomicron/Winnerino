#pragma once

#include "ExplorerPage.g.h"

namespace winrt::Winnerino::implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage>
    {
    public:
        ExplorerPage();
        ~ExplorerPage();

        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& args);

        void Page_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void TabView_TabCloseRequested(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args);
        void TabView_AddTabButtonClick(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void Page_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        winrt::event_token windowClosedToken;
        winrt::event_token loadingEventToken;

        void AddTab(hstring const& header, hstring const& path);
        void MainWindow_Closed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&);
        void SavePage();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage, implementation::ExplorerPage>
    {
    };
}
