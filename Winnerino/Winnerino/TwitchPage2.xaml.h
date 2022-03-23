#pragma once

#include "TwitchPage2.g.h"
#include <regex>

namespace winrt::Winnerino::implementation
{
    struct TwitchPage2 : TwitchPage2T<TwitchPage2>
    {
        TwitchPage2();

    public:
        void searchSuggestBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void webViewTabView_AddTabButtonClick(winrt::Microsoft::UI::Xaml::Controls::TabView const& sender, winrt::Windows::Foundation::IInspectable const& args);

    private:
        std::regex urlRegex;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct TwitchPage2 : TwitchPage2T<TwitchPage2, implementation::TwitchPage2>
    {
    };
}
