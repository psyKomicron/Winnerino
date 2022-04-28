#include "pch.h"
#include "FileTabView.xaml.h"
#if __has_include("FileTabView.g.cpp")
#include "FileTabView.g.cpp"
#endif
#include "shlwapi.h"
#include "fileapi.h"
#include "DirectorySizeCalculator.h"

using namespace winrt;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Storage;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;

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
        if (s.empty())
        {
            sender.Text(unbox_value<hstring>(args.SelectedItem()));
        }
        else
        {
            for (int16_t i = s.size() - 1; i > 0; i--)
            {
                if (s[i] == '\\')
                {
                    std::string substr = s.substr(0, i + 1);
                    sender.Text(to_hstring(substr) + unbox_value<hstring>(args.SelectedItem()));
                    return;
                }
            }
        }
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

    IAsyncAction FileTabView::listView_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
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
            else
            {
                // open the file with shell
                try
                {
                    hstring filePath = entry.FilePath();
                    StorageFile file = co_await StorageFile::GetFileFromPathAsync(filePath);
                    if (!co_await Launcher::LaunchFileAsync(file))
                    {
                        MainWindow::Current().NotifyUser(L"Failed to open file.", InfoBarSeverity::Warning);
                    }
                }
                catch (const hresult& ex)
                {
                    MainWindow::Current().NotifyError(ex);
                }
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
    }

    void FileTabView::forwardAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!forwardStack.empty())
        {
            hstring path = forwardStack.top();
            forwardStack.pop();
            backStack.push(path);
            loadPath(path);
        }
    }


    void FileTabView::loadPath(hstring path)
    {
        if (path == L"")
        {
#ifdef _DEBUG
            MainWindow::Current().NotifyUser(L"Path is empty (Debug)", InfoBarSeverity::Warning);
#endif // _DEBUG
            return;
        }

        if (PathFileExists(path.c_str()))
        {
            path = getRealPath(path); // get the real path with GetFinalPathNameByHandle, can return empty
            if (path == L"")
            {
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
                    hstring fileName = to_hstring(data.cFileName);
                    if ((fileName != L".." && fileName != L".") || showSpecialFolders())
                    {
                        WCHAR combinedPath[MAX_PATH]{ 0 };
                        PathCombine(combinedPath, path.c_str(), fileName.c_str());
                        hstring filePath = to_hstring(combinedPath);

                        int64_t size = static_cast<int64_t>(data.nFileSizeHigh) << 32;
                        size += data.nFileSizeLow;

                        listView().Items().Append(FileEntryView(fileName, filePath, size, data.dwFileAttributes));
                    }
                } while (FindNextFile(handle, &data));
                FindClose(handle);
            }
            else
            {
                MainWindow::Current().NotifyError(GetLastError());
            }

            progressRing().Visibility(Visibility::Collapsed);
            pathInputBox().Text(path);
        }
        else
        {
            MainWindow::Current().NotifyUser(L"Path not found \"" + path + L"\"", InfoBarSeverity::Error);
        }
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

    hstring FileTabView::getRealPath(hstring dirtyPath)
    {
        HANDLE fileHandle = CreateFile(dirtyPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            LPWSTR fullPath = NULL;
            DWORD stringLength = GetFinalPathNameByHandle(fileHandle, NULL, 0, VOLUME_NAME_DOS);
            if (stringLength > 0)
            {
                // malloc
                fullPath = new WCHAR[stringLength];
                stringLength = GetFinalPathNameByHandle(fileHandle, fullPath, stringLength, VOLUME_NAME_DOS);
                if (stringLength > 0)
                {
                    std::string s = to_string(fullPath).substr(4, s.size() - 4);
                    if (!s.ends_with("\\"))
                    {
                        s.append("\\");
                    }
                    // free resources before returning
                    delete[] fullPath;
                    CloseHandle(fileHandle);
                    return to_hstring(s);
                }
                else
                {
                    MainWindow::Current().NotifyUser(L"Failed to get full path.", InfoBarSeverity::Error);
                }

                // clean up
                delete[] fullPath;
            }
            else
            {
                MainWindow::Current().NotifyError(GetLastError());
            }
            // clean up
            CloseHandle(fileHandle);
        }
        else
        {
            MainWindow::Current().NotifyError(GetLastError());
        }

        return hstring{}; // if we reach this point, some error occured when trying to get the case sensitive path
    }

    bool FileTabView::showSpecialFolders()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        if (settings)
        {
            return unbox_value_or(settings.Values().TryLookup(L"ShowSpecialFolders"), false);
        }
        else
        {
            return false;
        }
    }
}