#include "pch.h"
#include "App.xaml.h"
#include "SettingsPage.xaml.h"
#include "MainWindow.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Data::Xml::Dom;

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
        PackageVersion version = Package::Current().Id().Version();
        AppVersionTextBlock().Text(L"Version " + to_hstring(version.Major) + L"." + to_hstring(version.Minor) + L"." + to_hstring(version.Build));
        hstring github = L"https://github.com/psykomicron/Winnerino";
        GithubHyperlinkButton().NavigateUri(Windows::Foundation::Uri{ github });
        //ToolTipService::SetToolTip(GithubHyperlinkButton(), box_value(github));

        InitSettings();
    }

    hstring SettingsPage::getCompactModeTooltip()
    {
        return L"Using compact overlay, the window will be more focused on the content and smaller.";
    }

#pragma region Event handlers
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

    IAsyncAction SettingsPage::openSettingsFile_Click(IInspectable const&, RoutedEventArgs const&)
    {
#if FALSE
        auto result = co_await contentDialog().ShowAsync();
        if (result == ContentDialogResult::Primary)
        {
            MainWindow::Current().NotifyUser(L"Exporting settings...", InfoBarSeverity::Informational);

            IMapView<hstring, ApplicationDataContainer> containersMap = ApplicationData::Current().LocalSettings().Containers();
            size_t max = containersMap.Size();
            if (max > 0)
            {
                XmlDocument doc{};
                for (auto pair : containersMap)
                {
                    XmlElement element = doc.CreateElement(pair.Key());
                    IPropertySet values = pair.Value().Values();
                    for (IKeyValuePair<hstring, IInspectable> propSet : values)
                    {
                        ApplicationDataCompositeValue composite = propSet.Value().try_as<ApplicationDataCompositeValue>();
                        if (composite)
                        {
                            XmlElement compositeValue = doc.CreateElement(propSet.Key());
                            element.AppendChild(compositeValue);
                            for (IKeyValuePair<hstring, IInspectable> compositePair : composite)
                            {

                            }
                        }
                    }

                    //element.NodeValue()
                }

                // creating file
                StorageFolder documents = KnownFolders::DocumentsLibrary();
                StorageFile file = co_await documents.CreateFileAsync(L"Exported settings", CreationCollisionOption::GenerateUniqueName);
                IRandomAccessStream stream = co_await file.OpenAsync(FileAccessMode::ReadWrite);
                DataWriter writer{ stream.GetOutputStreamAt(0) };

                co_await writer.StoreAsync();
                if (!co_await writer.FlushAsync())
                {
                    MainWindow::Current().NotifyUser(L"Failed to export settings", InfoBarSeverity::Error);
                }
            }
        }
#endif // FALSE
        co_return;
    }

    void SettingsPage::darkThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Dark);
    }

    void SettingsPage::lightThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Light);
    }

    void SettingsPage::defaultThemeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Default);
    }

    IAsyncAction SettingsPage::appDataFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try
        {
            co_await Launcher::LaunchFolderAsync(ApplicationData::Current().LocalFolder());
        }
        catch (hresult_error const&)
        {
            MainWindow::Current().NotifyUser(L"Failed to open appdata folder.", InfoBarSeverity::Warning);
        }
    }

    void SettingsPage::appSettingsFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Serialize all settings into a data type file and open it for the user ?

    }

    IAsyncAction SettingsPage::tempDataFolderHyperlink_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try
        {
            co_await Launcher::LaunchFolderAsync(ApplicationData::Current().TemporaryFolder());
        }
        catch (hresult_error const&)
        {
            MainWindow::Current().NotifyUser(L"Failed to open app temp folder.", InfoBarSeverity::Warning);
        }
    }

    void SettingsPage::clearTempFolderButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }

    void SettingsPage::clearSecureSettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }

    void SettingsPage::resetSettingsFile_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Clear();
    }

    void SettingsPage::compactOverlayToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        Winnerino::MainWindow window = Winnerino::MainWindow::Current();
        window.NotifyUser(L"Compact mode switched on", InfoBarSeverity::Informational);
    }

    void SettingsPage::ShowSpecialsFolderToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        if (!container)
        {
            container = ApplicationData::Current().LocalSettings().CreateContainer(L"Explorer", ApplicationDataCreateDisposition::Always);
        }
        container.Values().Insert(L"ShowSpecialFolders", box_value(ShowSpecialsFolderToggleSwitch().IsOn()));
    }

    void SettingsPage::CalculateDirectorySizeToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        if (!container)
        {
            container = ApplicationData::Current().LocalSettings().CreateContainer(L"Explorer", ApplicationDataCreateDisposition::Always);
        }
        container.Values().Insert(L"CalculateDirSize", box_value(CalculateDirectorySizeToggleSwitch().IsOn()));
    }

    void SettingsPage::loadLastPageToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        auto values = ApplicationData::Current().LocalSettings().Values();
        values.Insert(L"LoadLastPage", box_value(LoadLastPageToggleSwitch().IsOn()));
    }

    void SettingsPage::UseThumbnailsToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {

    }

    void winrt::Winnerino::implementation::SettingsPage::GoBackButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        MainWindow::Current().GoBack();
    }
#pragma endregion


    void SettingsPage::SwitchTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& requestedTheme)
    {
        MainWindow::Current().ChangeTheme(requestedTheme);
    }

    void SettingsPage::InitSettings()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();

        IInspectable inspectable = settings.Values().TryLookup(L"LoadLastPage");
        LoadLastPageToggleSwitch().IsOn(unbox_value_or<bool>(inspectable, true));
        inspectable = settings.Values().TryLookup(L"AppTheme");
        ElementTheme requestedTheme = unbox_value_or<ElementTheme>(inspectable, ElementTheme::Default);
        DarkThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Dark);
        LightThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Light);
        DefaultThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Default);

        // Explorer
        ApplicationDataContainer specificSettings = settings.Containers().TryLookup(L"Explorer");
        if (specificSettings)
        {
            inspectable = specificSettings.Values().TryLookup(L"ShowSpecialFolders");
            ShowSpecialsFolderToggleSwitch().IsOn(unbox_value_or<bool>(inspectable, false));
            inspectable = specificSettings.Values().TryLookup(L"CalculateDirSize");
            CalculateDirectorySizeToggleSwitch().IsOn(unbox_value_or(inspectable, true));
        }
        else
        {
            ShowSpecialsFolderToggleSwitch().IsOn(false);
            CalculateDirectorySizeToggleSwitch().IsOn(true);
        }
    }
}