#include "pch.h"
#include "TwitchPage2.xaml.h"
#if __has_include("TwitchPage2.g.cpp")
#include "TwitchPage2.g.cpp"
#endif
#include <iostream>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    TwitchPage2::TwitchPage2()
    {
        InitializeComponent();
        urlRegex = std::regex("^(https*)://");
    }

    void TwitchPage2::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        /*myButton().Content(box_value(L"Clicked"));*/
    }

    void TwitchPage2::searchSuggestBox_QuerySubmitted(AutoSuggestBox const& sender, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
    }

    void TwitchPage2::Page_Loaded(IInspectable const& sender, RoutedEventArgs const& e)
    {
        try
        {
            WebView2 tabContent = WebView2();
            tabContent.Source(Uri{ L"https://n-o-d-e.net/" });

            mainGrid().SetColumn(tabContent, 0);
            mainGrid().SetRow(tabContent, 1);

            mainGrid().Children().Append(tabContent);
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }

    void TwitchPage2::webViewTabView_AddTabButtonClick(TabView const& sender, IInspectable const& args)
    {
        
    }
}
