#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "MainWindow.g.h"

#pragma pop_macro("GetCurrentTime")

namespace winrt::Winnerino::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    private:
        static Winnerino::MainWindow singleton;
        bool isWindowFullscreen = false;
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        winrt::hstring lastPage;

        void initWindow();
        void saveWindowState();
        void updateInforBar(winrt::hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        bool loadPage(winrt::hstring page);

        void appWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const& args);
        void appWindow_Changed(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args);

    public:
        MainWindow();

        static Winnerino::MainWindow Current() { return singleton; }

        void notifyUser(winrt::hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        void notifyError(DWORD code);

        void navigationView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void navigationView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        void infoBar_Closed(winrt::Microsoft::UI::Xaml::Controls::InfoBar const& sender, winrt::Microsoft::UI::Xaml::Controls::InfoBarClosedEventArgs const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
