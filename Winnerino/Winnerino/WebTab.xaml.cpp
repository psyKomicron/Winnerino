#include "pch.h"
#include "WebTab.xaml.h"
#if __has_include("WebTab.g.cpp")
#include "WebTab.g.cpp"
#endif

using namespace std;
using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::Web::WebView2::Core;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    WebTab::WebTab()
    {
        InitializeComponent();
        webViewCoreInitializedToken = webView().CoreWebView2Initialized({ this, &WebTab::WebViewCore_Initialized });
    }

    WebTab::WebTab(hstring const& path, TabViewItem tab) : WebTab()
    {
        this->tab = tab;
        if (!path.empty())
        {
            NavigateTo(path);
        }
    }

    WebTab::~WebTab()
    {
        if (!tokenCleared)
        {
            webView().CoreWebView2().NavigationCompleted(webViewChangedToken);
        }
    }

    void WebTab::searchSuggestBox_QuerySubmitted(AutoSuggestBox const& sender, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        NavigateTo(searchSuggestBox().Text());
    }

    void WebTab::AppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        //webView().CoreWebView2().Stop();
        webView().CoreWebView2().NavigationCompleted(webViewChangedToken);
        tokenCleared = true;
        webView().Close();
    }

    void WebTab::NavigateTo(hstring const& url)
    {
        try
        {
            hstring destination = url;
            if (!regex_search(to_string(url), urlRegex))
            {
                destination = L"https://" + url;
            }
            webView().Source(Uri{ destination });
            searchSuggestBox().Text(destination);
        }
        catch (const hresult_error& ex)
        {
            MainWindow::Current().NotifyUser(ex.message(), InfoBarSeverity::Warning);
        }
    }

    void WebTab::WebViewCore_Initialized(WebView2 const& sender, CoreWebView2InitializedEventArgs const& args)
    {
        //webViewChangedToken = webView().NavigationCompleted({ this, &WebTab::WebView_NavigationCompleted });
        webViewChangedToken = webView().CoreWebView2().DocumentTitleChanged({ this, &WebTab::WebView_NavigationCompleted });
        webView().CoreWebView2Initialized(webViewCoreInitializedToken);
    }

    //void WebTab::WebView_NavigationCompleted(IInspectable const&, CoreWebView2NavigationCompletedEventArgs const& args)
    void WebTab::WebView_NavigationCompleted(CoreWebView2 const&, IInspectable const& args)
    {
        if (tab)
        {
            DispatcherQueue().TryEnqueue([this]()
            {
                tab.Header(box_value(webView().CoreWebView2().DocumentTitle()));
            });
        }
    }
}