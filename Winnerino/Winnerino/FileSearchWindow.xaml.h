#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime
#include "FileSearchWindow.g.h"
#pragma pop_macro("GetCurrentTime")

#include "DesktopTransparencyControllerType.h"

namespace winrt::Winnerino::implementation
{
    struct FileSearchWindow : FileSearchWindowT<FileSearchWindow>
    {
    public:
        FileSearchWindow();
        ~FileSearchWindow();

        void SetPosition(winrt::Windows::Graphics::PointInt32 const& position);

        void OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RootGrid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void AutoSuggestBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        Windows::Foundation::IAsyncAction CopyPathFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction OpenInExplorerFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ShowPropertiesFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        event_token appWindowClosingToken;
        event_token appWindowChangedEventToken;
        event_token mainWindowClosedEventToken;
        winrt::Windows::System::DispatcherQueueController dispatcherQueueController = nullptr;
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration systemBackdropConfiguration = nullptr;
        winrt::Microsoft::UI::Xaml::Window::Activated_revoker activatedRevoker;
        winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker themeChangedRevoker;
        winrt::Microsoft::UI::Composition::SystemBackdrops::ISystemBackdropController backdropController = nullptr;

        void InitWindow();
        void SetDragRectangles();
        void SetBackground(::Winnerino::DesktopTransparencyControllerType const& transparencyControllerType);
        void MainWindow_Closed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&);
        winrt::Windows::Foundation::IAsyncAction MatchFound(winrt::Windows::Foundation::IInspectable const& sender, winrt::hstring match);
        void AppWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const&);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileSearchWindow : FileSearchWindowT<FileSearchWindow, implementation::FileSearchWindow>
    {
    };
}
