#pragma once

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
        // static attributes
    private:
        static Winnerino::MainWindow singleton;

    private:
        bool isWindowFullscreen = false;
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        hstring lastPage;
        bool busy = false;
        std::queue<MessageData> messagesStack = std::queue<MessageData>();
        Microsoft::UI::Dispatching::DispatcherQueueTimer dispatcherQueueTimer = nullptr;

        // static methods
    public:
        static Winnerino::MainWindow Current() { return singleton; }

    public:
        MainWindow();
        /// <summary>
        /// Notifies the user with the specified message.
        /// </summary>
        /// <param name="message">Message to notify the user with</param>
        /// <param name="severity">Severity (info, success, warning, error)</param>
        void NotifyUser(hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        /// <summary>
        /// Notifies the user about an error. The method stringifies the code.
        /// </summary>
        /// <param name="code">Error code</param>
        void NotifyError(DWORD const& code);
        /// <summary>
        /// Notifies the user about an error, the method stringifies the code. The additional message will be prepended to the error code message.
        /// </summary>
        /// <param name="code">Error code</param>
        /// <param name="additionalMessage">Additional message to prepend to the error code message</param>
        void NotifyError(DWORD const& code, winrt::hstring const& additionalMessage);
        void ChangeTheme(Microsoft::UI::Xaml::ElementTheme const& theme);
        void navigationView_Loaded(Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void navigationView_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        void infoBar_Closed(Microsoft::UI::Xaml::Controls::InfoBar const& sender, Microsoft::UI::Xaml::Controls::InfoBarClosedEventArgs const& args);

    private:
        void InitWindow();
        void SaveWindowState();
        void UpdateInforBar(hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        bool LoadPage(winrt::hstring page);
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
