#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "Helper.h"

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
    //TODO: Move to recycle bin or delete files (ask for every file, give choice when deleting, show option...)
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
        PackageVersion version = Package::Current().Id().Version();
        AppVersionTextBlock().Text(L"Version " + to_hstring(version.Major) + L"." + to_hstring(version.Minor) + L"." + to_hstring(version.Build));
        hstring github = L"https://github.com/psykomicron/Winnerino";
        GithubHyperlinkButton().NavigateUri(Uri{ github });

        InitSettings();
    }

    hstring SettingsPage::getCompactModeTooltip()
    {
        return L"Using compact overlay, the window will be more focused on the content and smaller.";
    }

#pragma region Event handlers
    void SettingsPage::ShowSpecialsFolderToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ::Winnerino::get_or_create_container(L"Explorer");
        container.Values().Insert(L"ShowSpecialFolders", box_value(ShowSpecialsFolderToggleSwitch().IsChecked()));
    }

    void SettingsPage::CalculateDirectorySizeToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ::Winnerino::get_or_create_container(L"Explorer");
        container.Values().Insert(L"CalculateDirSize", box_value(CalculateDirectorySizeToggleSwitch().IsChecked()));
    }

    void SettingsPage::UseThumbnailsToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void winrt::Winnerino::implementation::SettingsPage::GoBackButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        //MainWindow::Current().GoBack();
        MainWindow::Current().NavigateTo(xaml_typename<ExplorerPage>());
    }

    void SettingsPage::Page_Unloaded(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().SizeChanged(mainWindowSizeChanged);
    }

    void SettingsPage::AdditionalSettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().NavigateTo(xaml_typename<AdditionalSettingsPage>());
    }

    void SettingsPage::NewContentButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().NavigateTo(xaml_typename<MainPage>());
    }

    void SettingsPage::HideSystemFilesCheckBox_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ::Winnerino::get_or_create_container(L"Explorer");
        container.Values().Insert(L"HideSystemFiles", box_value(HideSystemFilesCheckBox().IsChecked().GetBoolean()));
    }

    void winrt::Winnerino::implementation::SettingsPage::ShortcutMenuType_Checked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        ApplicationDataContainer container = ::Winnerino::get_or_create_container(L"Explorer");
        uint8_t type = 0;
        FrameworkElement element = sender.as<FrameworkElement>();
        if (element.Tag().as<hstring>() == L"ShortcutMenuType_Show")
        {
            type = 1;
        }
        container.Values().Insert(L"EntryShortcutMenuType", box_value(type));
    }

    void SettingsPage::MoveOrDeleteFileCheckBox_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ::Winnerino::get_or_create_container(L"Explorer").Values().Insert(L"MoveToRecyleBin", MoveOrDeleteFileCheckBox().IsChecked());
    }

    void SettingsPage::ShowSystemHealthToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer container = ::Winnerino::get_or_create_container(L"Explorer");
        container.Values().Insert(L"ShowSystemHealth", box_value(ShowSystemHealthToggleSwitch().IsOn()));
    }
#pragma endregion


    void SettingsPage::InitSettings()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();

        // Explorer settings
        {
            ApplicationDataContainer specificSettings = ::Winnerino::get_or_create_container(L"Explorer");

            ShowSpecialsFolderToggleSwitch().IsChecked(unbox_value_or<bool>(specificSettings.Values().TryLookup(L"ShowSpecialFolders"), false));
            CalculateDirectorySizeToggleSwitch().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"CalculateDirSize"), true));
            HideSystemFilesCheckBox().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"HideSystemFiles"), false));
            MoveOrDeleteFileCheckBox().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"MoveToRecyleBin"), false));

            UseThumbnailsToggleSwitch().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"UseThumbails"), false));
            ShowPathToggleSwitch().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"ShowFilePath"), false));

            ShowSystemHealthToggleSwitch().IsOn(unbox_value_or(specificSettings.Values().TryLookup(L"ShowSystemHealth"), true));

            uint8_t type = unbox_value_or(specificSettings.Values().TryLookup(L"EntryShortcutMenuType"), 0);
            switch (type)
            {
                case 0: // Quick menu none
                    SettingsShortcutMenuType_None().IsChecked(true);
                    break;
                case 1: // Quick menu normal
                    SettingsShortcutMenuType_Show().IsChecked(true);
                    break;
                case 2: // Quick menu detailed
                    SettingsShortcutMenuType_Show().IsChecked(true);
                    break;
            }
        }

        // File search settings
        {
            ApplicationDataContainer specificSettings = ::Winnerino::get_or_create_container(L"FileSearchWindow");

            FileSearchUsesRegex().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"UseRegex"), true));
            IncludeFoldersCheckBox().IsChecked(unbox_value_or(specificSettings.Values().TryLookup(L"IncludeFolders"), true));
        }
    }
}
