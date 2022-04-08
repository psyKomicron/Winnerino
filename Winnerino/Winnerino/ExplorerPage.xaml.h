#pragma once

#include "ExplorerPage.g.h"

namespace winrt::Winnerino::implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage>
    {
    private:

        void loadPath(winrt::hstring const& path);
        void completePath(winrt::hstring const& query, winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable> const& suggestions);
        winrt::Winnerino::FileEntryView getSelectedItem();

    public:
        ExplorerPage();
        void Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoSuggestBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        void AutoSuggestBox_TextChanged(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void AutoSuggestBox_GotFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AutoSuggestBox_SuggestionChosen(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args);
        void recentsButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void cutAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction copyAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void renameAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void deleteAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct ExplorerPage : ExplorerPageT<ExplorerPage, implementation::ExplorerPage>
    {
    };
}
