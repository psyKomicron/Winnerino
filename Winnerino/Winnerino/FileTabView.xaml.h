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
    private:
        std::stack<winrt::hstring> backStack = std::stack<winrt::hstring>();
        std::stack<winrt::hstring> forwardStack = std::stack<winrt::hstring>();

        void loadPath(winrt::hstring path);
        void completePath(winrt::hstring const& query, winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable> const& suggestions);
        hstring getRealPath(winrt::hstring dirtyPath);
        bool showSpecialFolders();

    public:
        FileTabView();
        FileTabView(winrt::hstring path);

        void pathInputBox_SuggestionChosen(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxSuggestionChosenEventArgs const& args);
        void pathInputBox_GotFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void pathInputBox_TextChanged(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args);
        void pathInputBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);
        winrt::Windows::Foundation::IAsyncAction listView_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
        void backAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void forwardAppBarButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileTabView : FileTabViewT<FileTabView, implementation::FileTabView>
    {
    };
}
