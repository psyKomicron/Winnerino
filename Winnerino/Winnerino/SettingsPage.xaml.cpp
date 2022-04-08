#include "pch.h"
#include "App.xaml.h"
#include "SettingsPage.xaml.h"
#include "MainWindow.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

using namespace Windows::Foundation;
using namespace Windows::Storage;

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

    void SettingsPage::settingSearch_QuerySubmitted(AutoSuggestBox const&, AutoSuggestBoxQuerySubmittedEventArgs const&)
    {

    }

    void SettingsPage::settingSearch_SuggestionChosen(AutoSuggestBox const&, AutoSuggestBoxSuggestionChosenEventArgs const&)
    {

    }

    void SettingsPage::settingSearch_TextChanged(AutoSuggestBox const&, AutoSuggestBoxTextChangedEventArgs const&)
    {

    }

    void SettingsPage::settingSearch_GotFocus(IInspectable const&, RoutedEventArgs const&)
    {

    }

    void SettingsPage::saveSettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().notifyUser(L"Saved settings.", InfoBarSeverity::Success);
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

    void SettingsPage::resetSettingsFile_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        
    }

    void SettingsPage::compactOverlayToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        Winnerino::MainWindow window = Winnerino::MainWindow::Current();
        window.notifyUser(L"Compact mode switched on", InfoBarSeverity::Informational);
    }

    void SettingsPage::loadLastPageToggleSwitch_Toggled(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto values = ApplicationData::Current().LocalSettings().Values();
        values.Insert(L"LoadLastPage", box_value(sender.as<ToggleSwitch>().IsOn()));
    }
}
