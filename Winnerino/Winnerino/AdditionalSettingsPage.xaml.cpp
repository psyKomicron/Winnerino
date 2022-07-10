#include "pch.h"
#include "AdditionalSettingsPage.xaml.h"
#if __has_include("AdditionalSettingsPage.g.cpp")
#include "AdditionalSettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    AdditionalSettingsPage::AdditionalSettingsPage()
    {
        InitializeComponent();
        InitSettings();
    }

    void AdditionalSettingsPage::ClearSecureSettingsButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {

    }

    void AdditionalSettingsPage::AnimatedIconToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {

    }

    void AdditionalSettingsPage::CompactOverlayToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        Winnerino::MainWindow window = Winnerino::MainWindow::Current();
    }

    IAsyncAction AdditionalSettingsPage::AppDataFolderHyperlink_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
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

    void AdditionalSettingsPage::AppSettingsFolderHyperlink_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        // Serialize all settings into a data type file and open it for the user ?
    }

    IAsyncAction AdditionalSettingsPage::TempDataFolderHyperlink_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
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

    void AdditionalSettingsPage::ClearTempFolderButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
    }

    void AdditionalSettingsPage::DarkThemeButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Dark);
    }

    void AdditionalSettingsPage::LightThemeButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Light);
    }

    void AdditionalSettingsPage::DefaultThemeButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        SwitchTheme(ElementTheme::Default);
    }

    void AdditionalSettingsPage::LoadLastPageToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto values = ApplicationData::Current().LocalSettings().Values();
        values.Insert(L"LoadLastPage", box_value(LoadLastPageToggleSwitch().IsChecked()));
    }

    IAsyncAction AdditionalSettingsPage::OpenSettingsFile_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
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

    void AdditionalSettingsPage::ResetSettingsFile_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Clear();
    }

    void AdditionalSettingsPage::BackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().GoBack();
    }

    void AdditionalSettingsPage::NotificationsSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"NotificationsEnabled", box_value(NotificationsSwitch().IsOn()));
    }

    void AdditionalSettingsPage::ShowSearchWindowInSwitchers_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"FileSearchWindow");
        settings.Values().Insert(L"ShowInSwitchers", box_value(ShowSearchWindowInSwitchers().IsChecked()));
    }

    void AdditionalSettingsPage::SearchWindowUsesAcrylic_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"FileSearchWindow");
        settings.Values().Insert(L"UsesAcrylic", box_value(SearchWindowUsesAcrylic().IsChecked()));
    }


    void AdditionalSettingsPage::SwitchTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& requestedTheme)
    {
        MainWindow::Current().ChangeTheme(requestedTheme);
    }

    void AdditionalSettingsPage::InitSettings()
    {
        const ApplicationDataContainer& settings = ApplicationData::Current().LocalSettings();

        LoadLastPageToggleSwitch().IsChecked(unbox_value_or<bool>(settings.Values().TryLookup(L"LoadLastPage"), true));
        NotificationsSwitch().IsOn(unbox_value_or<bool>(settings.Values().TryLookup(L"NotificationsEnabled"), true));

        IInspectable inspectable = settings.Values().TryLookup(L"AppTheme");
        ElementTheme requestedTheme = unbox_value_or<ElementTheme>(inspectable, ElementTheme::Default);
        DarkThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Dark);
        LightThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Light);
        DefaultThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Default);

        ApplicationDataContainer searchWindowSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"FileSearchWindow");
        if (!searchWindowSettings)
        {
            searchWindowSettings = ApplicationData::Current().LocalSettings().CreateContainer(L"FileSearchWindow", ApplicationDataCreateDisposition::Always);
        }
        
        SearchWindowUsesAcrylic().IsChecked(unbox_value_or<bool>(searchWindowSettings.Values().TryLookup((L"UsesAcrylic")), true));
        ShowSearchWindowInSwitchers().IsChecked(unbox_value_or<bool>(searchWindowSettings.Values().TryLookup((L"ShowInSwitchers")), false));

        // TODO: Implement settings for the file properties window
    }
}


void winrt::Winnerino::implementation::AdditionalSettingsPage::FilePropertiesWindowUsesAcrylic_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}


void winrt::Winnerino::implementation::AdditionalSettingsPage::ShowFilePropertiesWindowInSwitchers_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

}
