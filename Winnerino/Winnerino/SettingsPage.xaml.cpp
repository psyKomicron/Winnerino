#include "pch.h"
#include "App.xaml.h"
#include "SettingsPage.xaml.h"
#include "MainWindow.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
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
        InitSettings();
    }

    Uri SettingsPage::GithubUri()
    {
        return Windows::Foundation::Uri{ L"https://github.com/psykomicron/" };
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
        catch (hresult_error const& ex)
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
        catch (hresult_error const& ex)
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

    void SettingsPage::resetSettingsFile_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Clear();
    }

    void SettingsPage::compactOverlayToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        Winnerino::MainWindow window = Winnerino::MainWindow::Current();
        window.NotifyUser(L"Compact mode switched on", InfoBarSeverity::Informational);
    }

    void SettingsPage::UseSpecificLayoutToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TwitchSettings");
        container.Values().Insert(L"UseSpecificLayout", box_value(UseSpecificLayoutToggleSwitch().IsOn()));
    }

    void SettingsPage::UseAnimatedEmotesToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TwitchSettings");
        container.Values().Insert(L"UseAnimatedEmotes", box_value(UseAnimatedEmotesToggleSwitch().IsOn()));
    }

    void SettingsPage::ShowSpecialsFolderToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        container.Values().Insert(L"ShowSpecialFolders", box_value(ShowSpecialsFolderToggleSwitch().IsOn()));
    }

    void SettingsPage::CalculateDirectorySizeToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        container.Values().Insert(L"CalculateDirSize", box_value(CalculateDirectorySizeToggleSwitch().IsOn()));
    }

    void SettingsPage::loadLastPageToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        auto values = ApplicationData::Current().LocalSettings().Values();
        values.Insert(L"LoadLastPage", box_value(LoadLastPageToggleSwitch().IsOn()));
    }
#pragma endregion

    void SettingsPage::SwitchTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& requestedTheme)
    {
        darkThemeButton().IsChecked(requestedTheme == ElementTheme::Dark);
        lightThemeButton().IsChecked(requestedTheme == ElementTheme::Light);
        defaultThemeButton().IsChecked(requestedTheme == ElementTheme::Default);

        MainWindow::Current().ChangeTheme(requestedTheme);
    }

    void SettingsPage::InitSettings()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();

        IInspectable inspectable = settings.Values().TryLookup(L"LoadLastPage");
        LoadLastPageToggleSwitch().IsOn(unbox_value_or<bool>(inspectable, true));
        inspectable = settings.Values().TryLookup(L"AppTheme");
        SwitchTheme(unbox_value_or<ElementTheme>(inspectable, ElementTheme::Default));

        // Twitch
        ApplicationDataContainer specificSettings = settings.Containers().TryLookup(L"TwitchSettings");
        inspectable = specificSettings.Values().TryLookup(L"UseSpecificLayout");
        UseSpecificLayoutToggleSwitch().IsOn(unbox_value_or(inspectable, false));
        inspectable = specificSettings.Values().TryLookup(L"UseAnimatedEmotes");
        UseAnimatedEmotesToggleSwitch().IsOn(unbox_value_or(inspectable, true));
        inspectable = specificSettings.Values().TryLookup(L"ChatMention");
        ChatMentionTextBlock().Text(unbox_value_or<hstring>(inspectable, L""));
        inspectable = specificSettings.Values().TryLookup(L"EmoteSize");
        EmoteSizeSlider().Value(unbox_value_or<uint8_t>(inspectable, 0));
        inspectable = specificSettings.Values().TryLookup(L"ChatLength");
        ChatLengthSlider().Value(unbox_value_or<uint16_t>(inspectable, 500));

        // Explorer
        specificSettings = settings.Containers().TryLookup(L"Explorer");
        inspectable = specificSettings.Values().TryLookup(L"ShowSpecialFolders");
        ShowSpecialsFolderToggleSwitch().IsOn(unbox_value_or<bool>(inspectable, false));
        inspectable = specificSettings.Values().TryLookup(L"CalculateDirSize");
        CalculateDirectorySizeToggleSwitch().IsOn(unbox_value_or(inspectable, true));
    }
}
