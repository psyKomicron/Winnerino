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

        hstring ItemCount();
        hstring SelectedItemsCount();

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return m_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            m_propertyChanged.remove(token);
        };

        void PathInputBox_SuggestionChosen(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args);
        void PathInputBox_GotFocus(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PathInputBox_TextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void PathInputBox_QuerySubmitted(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        Windows::Foundation::IAsyncAction ListView_DoubleTapped(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
        void BackAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ForwardAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CutAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CopyAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RenameAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DeleteAppBarButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loaded(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RecentsButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const& sender, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args);
        void InputModeButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const& sender, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const& args);
        void FileListView_SelectionChanged(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void AlphabeticalSortButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SizeDescSortButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RefreshButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FavoriteButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearFavoritesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        hstring previousPath;
        std::stack<hstring> backStack = std::stack<hstring>();
        std::stack<hstring> forwardStack = std::stack<hstring>();
        event_token mainWindowClosedToken;
        hstring previousInput;
        Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter{ L"{hour.integer}:{minute.integer(2)} {month.integer(2)}/{day.integer(2)}/{year.abbreviated}" };
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        void CompletePath(hstring const& query, Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> const& suggestions);
        //int8_t Compare(FileEntryView const& that, FileEntryView const& other);
        inline hstring FormatFileSize(double* toFormat);
        hstring GetRealPath(hstring const& dirtyPath);
        void LoadPath(hstring path);
        void SavePage();
        void Search(hstring const& query, Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> const& suggestions);
        inline bool ShowSpecialFolders();
        void SortFiles();

        void MainWindow_Closed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileTabView : FileTabViewT<FileTabView, implementation::FileTabView>
    {
    };
}
