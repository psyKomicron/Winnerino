#include "pch.h"
#include "App.xaml.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

using namespace Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
    }

    Windows::Foundation::Uri SettingsPage::GithubUri()
    {
        return Windows::Foundation::Uri{ L"https://github.com/psykomicron/" };
    }

    winrt::hstring SettingsPage::getCompactModeTooltip()
    {
        return { L"Using compact overlay, the window will be more focused on the content and smaller." };
    }

    void SettingsPage::settingSearch_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {

    }


    void SettingsPage::settingSearch_SuggestionChosen(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args)
    {

    }


    void SettingsPage::settingSearch_TextChanged(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const&)
    {

    }


    void SettingsPage::settingSearch_GotFocus(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::saveSettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::openSettingsFile_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::darkThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::lightThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::defaultThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::appDataFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const& )
    {

    }


    void SettingsPage::appSettingsFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::tempDataFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::clearTempFolderButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void SettingsPage::clearSecureSettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }

    void SettingsPage::resetSettingsFile_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        
    }

    void SettingsPage::compactOverlayToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        winrt::Winnerino::implementation::App::MainWindow()
    }
}
