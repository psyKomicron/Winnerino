#pragma once

#include <regex>
#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "WebTab.g.h"

namespace winrt::Winnerino::implementation
{
    struct WebTab : WebTabT<WebTab>
    {
    private:
        std::regex urlRegex = std::regex("^(https*)://");
        winrt::event_token webViewChangedToken;
        bool tokenCleared;
        winrt::event_token webViewCoreInitializedToken;
        winrt::Microsoft::UI::Xaml::Controls::TabViewItem tab;

    public:
        WebTab();
        WebTab(winrt::hstring const& path, winrt::Microsoft::UI::Xaml::Controls::TabViewItem tab);
        ~WebTab();
        void searchSuggestBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        void AppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        void NavigateTo(hstring const& url);
        void WebViewCore_Initialized(winrt::Microsoft::UI::Xaml::Controls::WebView2 const& sender, winrt::Microsoft::UI::Xaml::Controls::CoreWebView2InitializedEventArgs const& args);
        //void WebView_NavigationCompleted(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::Web::WebView2::Core::CoreWebView2NavigationCompletedEventArgs const& args);
        void WebView_NavigationCompleted(winrt::Microsoft::Web::WebView2::Core::CoreWebView2 const& sender, IInspectable const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct WebTab : WebTabT<WebTab, implementation::WebTab>
    {
    };
}
