#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "FileTabView.g.h"
#include <stack>

namespace winrt::Winnerino::implementation
{
    struct FileTabView : FileTabViewT<FileTabView>
    {
    public:
        FileTabView();
        FileTabView(hstring path);
        ~FileTabView();

        inline hstring ItemCount(){ return to_hstring(FileListView().Items().Size()); };
        inline hstring SelectedItemsCount(){ return to_hstring(FileListView().SelectedItems().Size()); };
        inline Windows::Foundation::Collections::IObservableVector<Microsoft::UI::Xaml::Controls::UserControl> Files() { return _files; };

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return m_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            m_propertyChanged.remove(token);
        };

        void PathInputBox_SuggestionChosen(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args);
        void PathInputBox_GotFocus(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void PathInputBox_TextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void PathInputBox_QuerySubmitted(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        Windows::Foundation::IAsyncAction ListView_DoubleTapped(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const&);
        void BackButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void ForwardButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void CutAppBarButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void CopyAppBarButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void RenameAppBarButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void DeleteAppBarButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void UserControl_Loaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RecentsButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const&, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const&);
        void InputModeButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const&, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const&);
        void FileListView_SelectionChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&);
        void AlphabeticalSortButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SizeDescSortButton_Click(Windows::Foundation::IInspectable const& sder, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RefreshButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FavoriteButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearFavoritesButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SettingsButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void UpButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        hstring previousPath;
        std::stack<hstring> backStack = std::stack<hstring>();
        std::stack<hstring> forwardStack = std::stack<hstring>();
        event_token mainWindowClosedToken;
        hstring previousInput;
        Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter{ L"{hour.integer}:{minute.integer(2)} {month.integer(2)}/{day.integer(2)}/{year.abbreviated}" };
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        Windows::Foundation::Collections::IObservableVector<Microsoft::UI::Xaml::Controls::UserControl> _files{ single_threaded_observable_vector<Microsoft::UI::Xaml::Controls::UserControl>() };

        void CompletePath(hstring const& query, Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> const& suggestions);
        inline hstring FormatFileSize(double* toFormat) const;
        hstring GetRealPath(hstring const& dirtyPath);
        void LoadPath(hstring path);
        void SavePage();
        void Search(hstring const& query, Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> const& suggestions);
        inline bool ShowSpecialFolders();
        void SortFiles(const std::function<uint8_t(const FileEntryView&, const FileEntryView&)>& comparer);

        void MainWindow_Closed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileTabView : FileTabViewT<FileTabView, implementation::FileTabView>
    {
    };
}
