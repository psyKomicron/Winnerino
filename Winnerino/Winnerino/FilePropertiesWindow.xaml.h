#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime
#include "FilePropertiesWindow.g.h"
#pragma pop_macro("GetCurrentTime")

#include "DesktopTransparencyControllerType.h"

namespace winrt::Winnerino::implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow>
    {
    public:
        FilePropertiesWindow();
        ~FilePropertiesWindow();

        winrt::Windows::Foundation::IAsyncAction LoadFile(winrt::hstring file);
        winrt::Windows::Foundation::IAsyncAction LoadFolder(winrt::hstring folder);

        void Control_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RootGrid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);

    private:
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        event_token appWindowClosingToken;
        event_token appWindowChangedEventToken;
        event_token mainWindowClosedEventToken;
        event_token mainWindowThemeChangedToken;
        winrt::Windows::System::DispatcherQueueController dispatcherQueueController = nullptr;
        winrt::Microsoft::UI::Composition::SystemBackdrops::SystemBackdropConfiguration systemBackdropConfiguration = nullptr;
        winrt::Microsoft::UI::Xaml::Window::Activated_revoker activatedRevoker;
        winrt::Microsoft::UI::Xaml::FrameworkElement::ActualThemeChanged_revoker themeChangedRevoker;
        winrt::Microsoft::UI::Composition::SystemBackdrops::ISystemBackdropController backdropController = nullptr;

        void InitWindow();
        void SetBackground(::Winnerino::DesktopTransparencyControllerType const& transparencyControllerType);
        void OnAccessDenied(hstring path);
        void MainWindow_Closed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&);
        void AppWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const&);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow, implementation::FilePropertiesWindow>
    {
    };
}
