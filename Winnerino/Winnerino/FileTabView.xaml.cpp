#include "pch.h"
#include "FileTabView.xaml.h"
#if __has_include("FileTabView.g.cpp")
#include "FileTabView.g.cpp"
#include "shlwapi.h"
#include "fileapi.h"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileTabView::FileTabView()
    {
        InitializeComponent();
    }

    FileTabView::FileTabView(hstring path)
    {
        InitializeComponent();
        loadPath(path);
    }

    void FileTabView::pathInputBox_SuggestionChosen(AutoSuggestBox const& sender, AutoSuggestBoxSuggestionChosenEventArgs const& args)
    {
        std::string s = to_string(sender.Text());
#ifdef _DEBUG
        if (s.empty())
        {
            sender.Text(unbox_value<hstring>(args.SelectedItem()));
        }
        else
        {
            for (int i = s.size() - 1; i > 0; i--)
            {
                if (s[i] == '\\')
                {
                    auto substr = s.substr(0, i + 1);
                    sender.Text(to_hstring(substr) + unbox_value<hstring>(args.SelectedItem()));
                    return;
                }
            }
        }
#else
        for (int i = s.size() - 1; i > 0; i--)
        {
            if (s[i] == '\\')
            {
                auto substr = s.substr(0, i + 1);
                sender.Text(to_hstring(substr) + unbox_value<hstring>(args.SelectedItem()));
                return;
            }
        }
#endif // _DEBUG

    }

    void FileTabView::pathInputBox_GotFocus(IInspectable const& sender, RoutedEventArgs const&)
    {
        AutoSuggestBox box = sender.as<AutoSuggestBox>();
        IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
        completePath(box.Text(), suggestions);
        box.ItemsSource(suggestions);
    }

    void FileTabView::pathInputBox_TextChanged(AutoSuggestBox const& sender, AutoSuggestBoxTextChangedEventArgs const& args)
    {
        if (args.Reason() == AutoSuggestionBoxTextChangeReason::UserInput)
        {
            IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
            completePath(sender.Text(), suggestions);
            sender.ItemsSource(suggestions);
        }
    }

    void FileTabView::pathInputBox_QuerySubmitted(AutoSuggestBox const& sender, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        if (args.ChosenSuggestion())
        {
            hstring path = sender.Text() == L"" ? unbox_value_or<hstring>(args.ChosenSuggestion(), L"") : sender.Text();
            loadPath(path);
            if (!sender.Text().empty())
            {
                backStack.push(sender.Text());
            }
        }
        else
        {
            loadPath(args.QueryText());
            backStack.push(args.QueryText());
        }

    }

    void FileTabView::listView_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
    {
        FileEntryView entry = listView().SelectedItem().try_as<FileEntryView>();
        if (entry)
        {
            hstring path = pathInputBox().Text().ends_with(L"\\") ? pathInputBox().Text() : pathInputBox().Text() + L"\\";
            if (entry.IsDirectory())
            {
                backStack.push(path);
                loadPath(path + entry.FileName() + L"\\");
            }
        }
    }

    void FileTabView::backAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!backStack.empty())
        {
            hstring path = backStack.top();
            backStack.pop();
            forwardStack.push(path);
            loadPath(path);
        }
#ifdef _DEBUG
            MainWindow::Current().notifyUser(L"Back queue is empty", InfoBarSeverity::Warning);
#endif
    }

    void FileTabView::forwardAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {

    }


    void FileTabView::loadPath(hstring const& path)
    {
        if (path == L"")
        {
            MainWindow::Current().notifyUser(L"Path is empty (Debug)", InfoBarSeverity::Warning);
            return;
        }

        listView().Items().Clear();
        progressRing().Visibility(Visibility::Visible);

        WIN32_FIND_DATA data{};
        HANDLE handle = FindFirstFile((path + L"*").c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                int64_t size = static_cast<int64_t>(data.nFileSizeHigh) << 32;
                size += data.nFileSizeLow;
                listView().Items().Append(FileEntryView(to_hstring(data.cFileName), to_hstring(data.cAlternateFileName), size, data.dwFileAttributes));
            } while (FindNextFile(handle, &data));
            FindClose(handle);
        }
        else
        {
            MainWindow::Current().notifyError(GetLastError());
        }

        progressRing().Visibility(Visibility::Collapsed);
        pathInputBox().Text(path);
    }

    void FileTabView::completePath(hstring const& query, IVector<IInspectable> const& suggestions)
    {
        if (query.size() > 0)
        {
            // complete with FindFirstFile and FindNextFile
            WIN32_FIND_DATA data{};
            HANDLE findHandle = FindFirstFile((query + L"*").c_str(), &data);
            if (findHandle != INVALID_HANDLE_VALUE)
            {
                do
                {
                    int64_t size = static_cast<int64_t>(data.nFileSizeHigh) << 32;
                    size += data.nFileSizeLow;
                    if (to_hstring(data.cFileName) != L"." && to_hstring(data.cFileName) != L"..")
                    {
                        suggestions.Append(box_value(to_hstring(data.cFileName)));
                    }
                } while (FindNextFile(findHandle, &data));
                FindClose(findHandle);
            }
        }
        else
        {
            WCHAR drives[512]{};
            WCHAR* pointer = drives;
            GetLogicalDriveStrings(512, drives); // ignoring return value
            while (1)
            {
                if (*pointer == NULL)
                {
                    break;
                }
                suggestions.Append(box_value(hstring{ pointer }));
                while (*pointer++);
            }
        }
    }
}