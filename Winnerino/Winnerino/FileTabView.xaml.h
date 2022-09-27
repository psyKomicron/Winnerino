#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "FileTabView.g.h"
#include <stack>
#include "ToastGenerator.h"

namespace winrt::Winnerino::implementation
{
    struct FileTabView : FileTabViewT<FileTabView>
    {
    public:
        FileTabView();
        FileTabView(const hstring& path);
        ~FileTabView();

        inline hstring ItemCount() { return to_hstring(FilesListView().Items().Size()); };
        inline hstring SelectedItemsCount() { return to_hstring(FilesListView().SelectedItems().Size()); };
        inline Windows::Foundation::Collections::IObservableVector<Microsoft::UI::Xaml::FrameworkElement> Files() { return _files; };
        inline bool ProgressVisibility() { return _progressVisibility; };

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        void PathInputBox_SuggestionChosen(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args);
        void PathInputBox_GotFocus(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void PathInputBox_TextChanged(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void PathInputBox_QuerySubmitted(Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        void BackButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void ForwardButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void UserControl_Loaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RecentsButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const&, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const&);
        void InputModeButton_Click(Microsoft::UI::Xaml::Controls::SplitButton const&, Microsoft::UI::Xaml::Controls::SplitButtonClickEventArgs const&);
        void FileListView_SelectionChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&);
        void AlphabeticalSortButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SizeDescSortButton_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void RefreshButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FavoriteButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearFavoritesButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SettingsButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void UpButton_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OpenWithFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CopyPathFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SearchButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ShowPropertiesFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PathInputBox_PreviewKeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void ListViewFlyout_Opening(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void ListViewButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        winrt::Windows::Foundation::IAsyncAction GridViewButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction FilesGridView_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction OpenInExplorerFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction ListView_DoubleTapped(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const&);
        winrt::Windows::Foundation::IAsyncAction CutFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction CopyFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction RenameFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction DeleteFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction SpotlightImporter_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        winrt::hstring previousPath{};
        std::stack<hstring> backStack = std::stack<hstring>();
        std::stack<hstring> forwardStack = std::stack<hstring>();
        std::vector<Winnerino::FileEntryView::Deleted_revoker> fileEntryDeletedRevokers{};
        winrt::event_token windowClosedToken;
        winrt::event_token windowSizeChangedToken;
        winrt::event_token loadingEventToken;
        ::Winnerino::Common::ToastGenerator toastGenerator{};
        bool listingDrives = false;
        bool tabRenamed = false;
        bool gridViewLoaded = false;
        bool listViewLoaded = false;
        bool _progressVisibility = false;
        Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter
        {
            L"{hour.integer}:{minute.integer(2)} {month.integer(2)}/{day.integer(2)}/{year.abbreviated}"
        };
        Windows::Foundation::Collections::IObservableVector<Microsoft::UI::Xaml::FrameworkElement> _files
        { 
            single_threaded_observable_vector<Microsoft::UI::Xaml::FrameworkElement>()
        };

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        void CompletePath(winrt::hstring const& query, winrt::Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable>* const& suggestions);
        winrt::hstring GetRealPath(winrt::hstring const& dirtyPath);
        inline void GetOptions(bool* showSpecialFolders, bool* hideSystemFiles);
        void LoadPath(hstring path);
        Windows::Foundation::IAsyncAction LoadToGridView(winrt::hstring path);
        Windows::Foundation::IAsyncAction Open(winrt::hstring const& info);
        void ProgressVisibility(bool const& value);
        void SavePage();
        void Search(winrt::hstring const& query, winrt::Windows::Foundation::Collections::IVector<Windows::Foundation::IInspectable> const& suggestions);
        void SortFiles(const std::function<uint8_t(const winrt::Winnerino::FileEntryView&, const winrt::Winnerino::FileEntryView&)>& comparer);
        void SetLayout(float const& width);
        void GoForward();
        void GoBack();

        void Window_SizeChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowSizeChangedEventArgs const& args);
        void MainWindow_Closed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileTabView : FileTabViewT<FileTabView, implementation::FileTabView>
    {
    };
}
