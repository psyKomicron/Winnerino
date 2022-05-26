#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

#include "App.xaml.h"
#include "MainWindow.xaml.h"

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
    SettingsPage::SettingsPage()
    {
        InitializeComponent();
        PackageVersion version = Package::Current().Id().Version();
        AppVersionTextBlock().Text(L"Version " + to_hstring(version.Major) + L"." + to_hstring(version.Minor) + L"." + to_hstring(version.Build));
        hstring github = L"https://github.com/psykomicron/Winnerino";
        GithubHyperlinkButton().NavigateUri(Uri{ github });

        SetLayout(MainWindow::Current().Size().Width);
        mainWindowSizeChanged = MainWindow::Current().SizeChanged({ this, &SettingsPage::Window_SizeChanged });

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

    void SettingsPage::UseThumbnailsToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void winrt::Winnerino::implementation::SettingsPage::GoBackButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        MainWindow::Current().GoBack();
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
#pragma endregion


    void SettingsPage::InitSettings()
    {
        // Explorer
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();
        ApplicationDataContainer specificSettings = settings.Containers().TryLookup(L"Explorer");
        if (specificSettings)
        {
            IInspectable inspectable = specificSettings.Values().TryLookup(L"ShowSpecialFolders");
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

    void SettingsPage::SetLayout(float const& width)
    {
        if (width < 800)
        {
            if (Grid::GetRow(AboutGrid()) != 2)
            {
                Grid::SetRow(AboutGrid(), 2);
            }

            if (Grid::GetColumn(AboutGrid()) != 0)
            {
                Grid::SetColumn(AboutGrid(), 0);
                Grid::SetColumnSpan(AboutGrid(), 2);
            }

            if (Grid::GetColumnSpan(SettingsGrid()) != 2)
            {
                Grid::SetColumnSpan(SettingsGrid(), 2);
                Grid::SetRowSpan(SettingsGrid(), 1);
            }
        }
        else
        {
            if (Grid::GetRow(AboutGrid()) != 0)
            {
                Grid::SetRow(AboutGrid(), 0);
            }

            if (Grid::GetColumn(AboutGrid()) != 1)
            {
                Grid::SetColumn(AboutGrid(), 1);
                Grid::SetColumnSpan(AboutGrid(), 1);
            }

            if (Grid::GetColumnSpan(SettingsGrid()) != 1)
            {
                Grid::SetColumnSpan(SettingsGrid(), 1);
                Grid::SetRowSpan(SettingsGrid(), 2);
            }
        }
    }

    void SettingsPage::Window_SizeChanged(IInspectable const&, WindowSizeChangedEventArgs const& args)
    {
        SetLayout(args.Size().Width);
    }
}