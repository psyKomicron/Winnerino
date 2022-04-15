#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "MainWindow.g.h"
#include <queue>
#include "MessageData.h"

#pragma pop_macro("GetCurrentTime")

using namespace winrt;
using namespace std;

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
        queue<MessageData> messagesStack = queue<MessageData>();
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer dispatcherQueueTimer = nullptr;

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
        void navigationView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void navigationView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
        void infoBar_Closed(winrt::Microsoft::UI::Xaml::Controls::InfoBar const& sender, winrt::Microsoft::UI::Xaml::Controls::InfoBarClosedEventArgs const& args);

    private:
        void initWindow();
        void saveWindowState();
        void updateInforBar(winrt::hstring const& message, Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        bool loadPage(winrt::hstring page);
        void appWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const& args);
        void appWindow_Changed(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args);
        void dispatcherQueueTimer_Tick(Microsoft::UI::Dispatching::DispatcherQueueTimer const& timer, Windows::Foundation::IInspectable const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
