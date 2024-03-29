﻿#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "MainWindow.g.h"
#include <queue>
#include "MessageData.h"

#pragma pop_macro("GetCurrentTime")

#define USING_TIMER 0

namespace winrt::Winnerino::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        // static methods
    public:
        MainWindow();

        static Winnerino::MainWindow Current() { return singleton; }
        Windows::Foundation::Size Size() const;
        winrt::Microsoft::UI::WindowId Id() const { return appWindow.Id(); };
        winrt::Windows::Graphics::PointInt32 Position() { return appWindow.Position(); };

        void NotifyUser(hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        void NotifyError(DWORD const& code);
        void NotifyError(DWORD const& code, winrt::hstring const& additionalMessage);
        void ChangeTheme(Microsoft::UI::Xaml::ElementTheme const& theme);
        bool NavigateTo(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName);
        bool NavigateTo(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName, Windows::Foundation::IInspectable const& parameter);
        void GoBack();

        event_token ThemeChanged(Windows::Foundation::TypedEventHandler<Microsoft::UI::Xaml::Window, Microsoft::UI::Xaml::ElementTheme> const& handler)
        {
            return e_themeChanged.add(handler);
        };
        void ThemeChanged(event_token const& token)
        {
            e_themeChanged.remove(token);
        };

        void View_Loaded(Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void navigationView_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        void InfoBar_Closed(Microsoft::UI::Xaml::Controls::InfoBar const& sender, Microsoft::UI::Xaml::Controls::InfoBarClosedEventArgs const& args);
        void ContentFrame_NavigationFailed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationFailedEventArgs const& e);

    private:
        static Winnerino::MainWindow singleton;
        bool isWindowFullscreen = false;
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        hstring lastPage;
        bool busy = false;
        std::queue<MessageData> messagesStack = std::queue<MessageData>();
        Microsoft::UI::Dispatching::DispatcherQueueTimer dispatcherQueueTimer = nullptr;
        event<Windows::Foundation::TypedEventHandler<Microsoft::UI::Xaml::Window, Microsoft::UI::Xaml::ElementTheme>> e_themeChanged;

        void InitWindow();
        void SaveWindowState();
        void UpdateInforBar(hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        bool LoadPage(winrt::hstring page);
        void SetDragRectangles();
        void appWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const& args);
        void appWindow_Changed(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args);
#if USING_TIMER
        void dispatcherQueueTimer_Tick(Microsoft::UI::Dispatching::DispatcherQueueTimer const& timer, Windows::Foundation::IInspectable const& args);
#endif // USING_TIMER
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
