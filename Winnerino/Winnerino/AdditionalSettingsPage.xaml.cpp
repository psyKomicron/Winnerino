#include "pch.h"
#include "AdditionalSettingsPage.xaml.h"
#if __has_include("AdditionalSettingsPage.g.cpp")
#include "AdditionalSettingsPage.g.cpp"
#endif

#include "DesktopTransparencyControllerType.h"
#include "Helper.h"

using namespace ::Winnerino;
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


    void AdditionalSettingsPage::OnPageLoaded(IInspectable const&, RoutedEventArgs const&)
    {
        //InitSettings();

        bool isOn = UseAcrylicToggleSwitch().IsOn();
        SearchWindowTransparencyEffects().IsEnabled(isOn);
        FilePropsWindowTransparencyEffects().IsEnabled(isOn);
    }


    void AdditionalSettingsPage::ClearSecureSettingsButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        // TODO: Allow user to clear secure settings (even if none right now v2.1.0).
    }

    void AdditionalSettingsPage::AnimatedIconToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        // TODO: Implement animated window icon control (on/off).
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
        // TODO: Allow user to clear temporary files (even if none right now v2.1.0)
        //ApplicationData::Current().TemporaryFolder().GetParentAsync()
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

    void AdditionalSettingsPage::KeepSearchWindowOnTop_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().Lookup(L"FileSearchWindow");
        settings.Values().Insert(L"KeepOnTop", box_value(KeepSearchWindowOnTop().IsChecked()));
    }

    void AdditionalSettingsPage::SearchWindowUsesAcrylic_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().Lookup(L"FileSearchWindow");
        settings.Values().Insert(L"UsesAcrylic", box_value(SearchWindowTransparencyEffects().IsOn()));
    }

    void AdditionalSettingsPage::FilePropertiesWindowUsesAcrylic_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().Lookup(L"FilePropertiesWindow");
        settings.Values().Insert(L"UsesAcrylic", box_value(SearchWindowTransparencyEffects().IsOn()));
    }

    void AdditionalSettingsPage::KeepFilePropertiesWindowOnTop_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().Lookup(L"FilePropertiesWindow");
        settings.Values().Insert(L"KeepOnTop", box_value(KeepFilePropertiesWindowOnTop().IsChecked()));
    }

    void AdditionalSettingsPage::UseAcrylicCheckBox_Click(IInspectable const&, RoutedEventArgs const&)
    {
        bool isOn = UseAcrylicToggleSwitch().IsOn();
        ApplicationData::Current().LocalSettings().Values().Insert(L"WindowsCanUseTransparencyEffects", IReference(isOn));
        SearchWindowTransparencyEffects().IsEnabled(isOn);
        FilePropsWindowTransparencyEffects().IsEnabled(isOn);
    }

    void AdditionalSettingsPage::MicaRadioButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"TransparencyEffectController", box_value((uint8_t)DesktopTransparencyControllerType::Mica));
    }

    void AdditionalSettingsPage::AcrylicRadioButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationData::Current().LocalSettings().Values().Insert(L"TransparencyEffectController", box_value((uint8_t)DesktopTransparencyControllerType::Acrylic));
    }


    void AdditionalSettingsPage::SwitchTheme(winrt::Microsoft::UI::Xaml::ElementTheme const& requestedTheme)
    {
        MainWindow::Current().ChangeTheme(requestedTheme);
    }

    void AdditionalSettingsPage::InitSettings()
    {
        const ApplicationDataContainer& settings = ApplicationData::Current().LocalSettings();

        // App theme
        {
            IInspectable inspectable = settings.Values().TryLookup(L"AppTheme");
            ElementTheme requestedTheme = unbox_value_or(inspectable, ElementTheme::Default);
            DarkThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Dark);
            LightThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Light);
            DefaultThemeRadioButton().IsChecked(requestedTheme == ElementTheme::Default);
        }

        // Transparency settings
        {
            IInspectable inspectable = settings.Values().TryLookup(L"TransparencyEffectController");
            DesktopTransparencyControllerType transparencyController = (DesktopTransparencyControllerType)unbox_value_or(
                inspectable,
                (uint8_t)DesktopTransparencyControllerType::Acrylic
                );
            AcrylicRadioButton().IsChecked(transparencyController == DesktopTransparencyControllerType::Acrylic);
            MicaRadioButton().IsChecked(transparencyController == DesktopTransparencyControllerType::Mica);
        }

        LoadLastPageToggleSwitch().IsChecked(unbox_value_or(settings.Values().TryLookup(L"LoadLastPage"), true));
        NotificationsSwitch().IsOn(unbox_value_or(settings.Values().TryLookup(L"NotificationsEnabled"), true));
        UseAcrylicToggleSwitch().IsOn(unbox_value_or(settings.Values().TryLookup(L"WindowsCanUseTransparencyEffects"), true));

        // Search window settings
        {
            ApplicationDataContainer searchWindowSettings = get_or_create_container(L"FileSearchWindow");
            SearchWindowTransparencyEffects().IsOn(unbox_value_or(searchWindowSettings.Values().TryLookup((L"UsesAcrylic")), true));
            KeepSearchWindowOnTop().IsChecked(unbox_value_or(searchWindowSettings.Values().TryLookup((L"KeepOnTop")), true));
        }

        // File properties window settings
        {
            ApplicationDataContainer filePropsWindow = get_or_create_container(L"FilePropertiesWindow");
            FilePropsWindowTransparencyEffects().IsOn(unbox_value_or(filePropsWindow.Values().TryLookup(L"UsesAcrylic"), true));
            KeepFilePropertiesWindowOnTop().IsChecked(unbox_value_or(filePropsWindow.Values().TryLookup(L"KeepOnTop"), true));
        }
    }

    void AdditionalSettingsPage::TransparencyControlsEnabled(bool const& value)
    {
        _transparencyControlsEnabled = value;
        e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(L"TransparencyControlsEnabled"));
    }
}