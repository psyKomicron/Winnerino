#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "FilePropertiesWindow.g.h"

#pragma pop_macro("GetCurrentTime")

namespace winrt::Winnerino::implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow>
    {
    public:
        FilePropertiesWindow();
        ~FilePropertiesWindow();

        void TitleBarGrid_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LocalTabView_TabCloseRequested(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args);
        void LocalTabView_AddTabButtonClick(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;
        event_token appWindowChangedEventToken;

        void OnWindowChanged(winrt::Microsoft::UI::Windowing::AppWindow const&, winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args);
        void InitWindow();
        void SetDragRectangles();
    public:
        void TabStripFooter_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow, implementation::FilePropertiesWindow>
    {
    };
}
