#include "pch.h"
#include "FileTabView.xaml.h"
#if __has_include("FileTabView.g.cpp")
#include "FileTabView.g.cpp"
#endif
#include <chrono>
#include <wchar.h>
#include <regex>
#include "shlwapi.h"
#include "fileapi.h"
#include "DirectorySizeCalculator.h"
#include "QuickSort.h"

using namespace std;
using namespace winrt;

using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;

using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Globalization::DateTimeFormatting;
using namespace winrt::Windows::Security::Cryptography;
using namespace winrt::Windows::Security::Cryptography::Core;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileTabView::FileTabView()
    {
        InitializeComponent();
        mainWindowClosedToken = MainWindow::Current().Closed({ this, &FileTabView::MainWindow_Closed });
    }

    FileTabView::FileTabView(hstring path)
    {
        InitializeComponent();
        mainWindowClosedToken = MainWindow::Current().Closed({ this, &FileTabView::MainWindow_Closed });
        LoadPath(path);
    }

    FileTabView::~FileTabView()
    {
        MainWindow::Current().Closed(mainWindowClosedToken);
    }

    winrt::hstring FileTabView::ItemCount()
    {
        return to_hstring(FileListView().Items().Size());
    }

    winrt::hstring FileTabView::SelectedItemsCount()
    {
        return to_hstring(FileListView().SelectedItems().Size());
    }


    void FileTabView::PathInputBox_SuggestionChosen(AutoSuggestBox const& sender, AutoSuggestBoxSuggestionChosenEventArgs const& args)
    {
        std::string s = to_string(sender.Text());
        if (s.empty())
        {
            optional<hstring> opt = args.SelectedItem().try_as<hstring>();
            if (opt)
            {
                sender.Text(unbox_value<hstring>(args.SelectedItem()));
            }
            else
            {
                optional<FrameworkElement> element = args.SelectedItem().try_as<FrameworkElement>();
                if (element)
                {
                    sender.Text(element.Value().FindName(L"PathTextBlock").as<TextBlock>().Text());
                }
            }
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

    void FileTabView::PathInputBox_GotFocus(IInspectable const& sender, RoutedEventArgs const&)
    {
        AutoSuggestBox box = sender.as<AutoSuggestBox>();
        IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
        if (InputModeButton().IsChecked())
        {
            Search(box.Text(), suggestions);
        }
        else
        {
            CompletePath(box.Text(), suggestions);
        }
        box.ItemsSource(suggestions);
    }

    void FileTabView::PathInputBox_TextChanged(AutoSuggestBox const& sender, AutoSuggestBoxTextChangedEventArgs const& args)
    {
        if (args.Reason() == AutoSuggestionBoxTextChangeReason::UserInput)
        {
            IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
            if (InputModeButton().IsChecked())
            {
                Search(sender.Text(), suggestions);
            }
            else
            {
                CompletePath(sender.Text(), suggestions);
            }
            sender.ItemsSource(suggestions);
        }
    }

    void FileTabView::PathInputBox_QuerySubmitted(AutoSuggestBox const&, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        if (InputModeButton().IsChecked())
        {
            hstring path = previousInput + unbox_value_or<hstring>(args.ChosenSuggestion(), L"");
            LoadPath(path);
        }
        else
        {
            if (!previousPath.empty())
            {
                backStack.push(previousPath);
            }
            LoadPath(args.QueryText());
        }
    }

    IAsyncAction FileTabView::ListView_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
    {
        FileEntryView entry = FileListView().SelectedItem().try_as<FileEntryView>();
        if (entry)
        {
            if (entry.IsDirectory())
            {
                if (!previousPath.empty())
                {
                    backStack.push(previousPath);
                }
                LoadPath(entry.FilePath() + L"\\");
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

    void FileTabView::FileListView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const& e)
    {
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"SelectedItemsCount" });

        if (FileListView().SelectedItems().Size() == 1)
        {
            FileEntryView view = FileListView().SelectedItem().try_as<FileEntryView>();
            FileOpenAppTextBlock().Text(view.OpensWith());
            FileSizeTextBlock().Text(view.FileSize());
            FilePerceivedTypeTextBlock().Text(view.PerceivedType());

            FileLastWriteTextBlock().Text(formatter.Format(view.LastWrite()));
            FileDangerousFontIcon().Visibility(view.IsFileDangerous() ? Visibility::Visible : Visibility::Collapsed);
        }
        else
        {
            FileOpenAppTextBlock().Text(L"");
            auto&& selectedItems = FileListView().SelectedItems();
            uint64_t combinedSize = 0;
            for (auto&& selectedItem : selectedItems)
            {
                FileEntryView view = selectedItem.try_as<FileEntryView>();
                if (view)
                {
                    combinedSize += view.FileBytes();
                }
            }
            double size = static_cast<double>(combinedSize);
            hstring ext = FormatFileSize(&size);
            FileSizeTextBlock().Text(to_hstring(size) + ext);
        }
    }

    void FileTabView::BackAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!backStack.empty())
        {
            hstring path = backStack.top();
            backStack.pop();
            forwardStack.push(previousPath);
            LoadPath(path);
        }
    }

    void FileTabView::ForwardAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!forwardStack.empty())
        {
            hstring path = forwardStack.top();
            forwardStack.pop();
            backStack.push(path);
            LoadPath(path);
        }
    }

    void FileTabView::CutAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().NotifyUser(L"Cutting files to clipboard is not supported yet.", InfoBarSeverity::Error);
    }

    void FileTabView::CopyAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        DataPackage dataPackage{};
        IVector<IStorageItem> items{};

        dataPackage.SetStorageItems(items);
        dataPackage.RequestedOperation(DataPackageOperation::Copy);
        Clipboard::SetContent(dataPackage);
        MainWindow::Current().NotifyUser(L"File(s) copied to clipboard.", InfoBarSeverity::Success);
    }

    void FileTabView::RenameAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
    }

    void FileTabView::DeleteAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
    }

    void FileTabView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        std::optional<bool> isChecked = settings.Values().Lookup(L"UseSearchRegex").try_as<bool>();
        UseRegexButton().IsChecked(isChecked.value_or(false));

        ApplicationDataContainer recents = settings.Containers().TryLookup(L"ExplorerRecents");
        if (recents)
        {
            auto containers = recents.Values();
            for (auto const& c : containers)
            {
                hstring recent = unbox_value_or<hstring>(c.Value(), L"");
                if (recent != L"")
                {
                    AppBarButton item{};
                    item.Label(recent);
                    item.Tag(box_value(recent));
                    FileCommandBar().SecondaryCommands().Append(item);
                }
            }
        }

        // list available drives
#if FALSE
        WCHAR drives[512]{};
        WCHAR* pointer = drives;
        GetLogicalDriveStrings(512, drives);
        while (1)
        {
            if (*pointer == NULL)
            {
                break;
            }
            DriveListView().Items().Append(Winnerino::DriveSearchModel{ hstring{ pointer } });
            while (*pointer++);
        }
#endif // FALSE

    }

    void FileTabView::RecentsButton_Click(SplitButton const&, SplitButtonClickEventArgs const&)
    {
#if FALSE
        hstring currentPath = PathInputBox().Text();

        if (currentPath == L"")
        {
            return;
        }
        // find if duplicate (watch out for StackOverflow mods)
        auto items = RecentsMenuFlyout().Items();
        for (auto const& entry : items)
        {
            MenuFlyoutItem flyout = entry.try_as<MenuFlyoutItem>();
            if (flyout && flyout.Text() == currentPath)
            {
                return;
            }
        }

        // save recent
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings();
        ApplicationDataContainer recents = container.Values().TryLookup(L"ExplorerRecents").try_as<ApplicationDataContainer>();

        IBuffer buffer = CryptographicBuffer::ConvertStringToBinary(currentPath, BinaryStringEncoding::Utf8);
        HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
        hstring key = CryptographicBuffer::EncodeToHexString(provider.HashData(buffer));

        if (!recents)
        {
            recents = container.CreateContainer(L"ExplorerRecents", ApplicationDataCreateDisposition::Always);
        }
        recents.Values().Insert(key, box_value(currentPath));

        // add to view
        MenuFlyoutItem entry{};
        entry.Text(currentPath);
        RecentsMenuFlyout().Items().Append(entry);
#endif // FALSE

    }

    void FileTabView::InputModeButton_Click(SplitButton const&, SplitButtonClickEventArgs const&)
    {
        if (InputModeButton().IsChecked())
        {
            previousInput = PathInputBox().Text();
            PathInputBox().Text(L"");
            PathInputBox().PlaceholderText(L"Search");
        }
        else
        {
            PathInputBox().Text(previousInput);
            PathInputBox().PlaceholderText(L"Path");
        }
    }

    void FileTabView::AlphabeticalSortButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw hresult_not_implemented();
    }

    void FileTabView::SizeDescSortButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        SortFiles();
    }

    void FileTabView::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        LoadPath(previousPath);
    }

    void FileTabView::FavoriteButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!previousInput.empty())
        {
            ApplicationDataContainer explorerSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
            if (explorerSettings)
            {
                ApplicationDataContainer favorites = explorerSettings.Containers().TryLookup(L"Favorites");
                if (!favorites)
                {
                    favorites = explorerSettings.CreateContainer(L"Favorites", ApplicationDataCreateDisposition::Always);
                }

                IPropertySet values = favorites.Values();
                for (auto&& value : values)
                {
                    const optional<hstring>& optional = value.try_as<hstring>();
                    if (optional && optional.Value() == previousPath)
                    {
                        return;
                    }
                }

                HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
                hstring key = CryptographicBuffer::EncodeToHexString(
                    provider.HashData(CryptographicBuffer::ConvertStringToBinary(previousPath, BinaryStringEncoding::Utf8)));
                favorites.Values().Insert(key, box_value(previousInput));
                // PathInputBox().Text()
            }
        }
    }

    void FileTabView::ClearFavoritesButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer explorerSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        explorerSettings.DeleteContainer(L"Favorites");
    }


    void FileTabView::CompletePath(hstring const& query, IVector<IInspectable> const& suggestions)
    {
        if (!query.empty())
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

                    wchar_t dot[2]{ '.', '\0' };
                    wchar_t dotdot[3]{ '.', '.', '\0' };
                    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && wcscmp(data.cFileName, dot) != 0 && wcscmp(data.cFileName, dotdot) != 0)
                    {
                        suggestions.Append(box_value(to_hstring(data.cFileName)));
                    }
                } while (FindNextFile(findHandle, &data));
                FindClose(findHandle);
            }
        }
        else
        {
            ApplicationDataContainer explorerSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
            if (explorerSettings)
            {
                ApplicationDataContainer favorites = explorerSettings.Containers().TryLookup(L"Favorites");
                if (favorites)
                {
                    IPropertySet propSet = favorites.Values();
                    for (auto&& prop : propSet)
                    {
                        optional<hstring> path = prop.Value().try_as<hstring>();
                        if (path)
                        {
                            Grid grid{};

                            ColumnDefinition colDef{};
                            
                            GridLength width = GridLength{ 1 };
                            width.GridUnitType = GridUnitType::Star;
                            colDef.Width(width);
                            grid.ColumnDefinitions().Append(colDef);

                            width = GridLength{};
                            width.GridUnitType = GridUnitType::Auto;
                            colDef = ColumnDefinition{};
                            colDef.Width(width);
                            grid.ColumnDefinitions().Append(colDef);

                            FontIcon fontIcon{};
                            fontIcon.VerticalAlignment(VerticalAlignment::Center);
                            fontIcon.Glyph(L"\ue728");

                            TextBlock textBlock{};
                            textBlock.Name(L"PathTextBlock");
                            textBlock.VerticalAlignment(VerticalAlignment::Center);
                            textBlock.Text(path.Value());

                            Grid::SetColumn(textBlock, 0);
                            Grid::SetColumn(fontIcon, 1);

                            grid.Children().Append(fontIcon);
                            grid.Children().Append(textBlock);

                            suggestions.Append(grid);
                        }
                    }
                }
            }

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

    inline winrt::hstring FileTabView::FormatFileSize(double* size)
    {
        if (*size >= 0x10000000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x10000000000);
            *size /= (double)100;
            return L" Tb";
        }
        if (*size >= 0x40000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x40000000);
            *size /= (double)100;
            return L" Gb";
        }
        if (*size >= 0x100000)
        {
            *size *= (double)100;
            *size = round(*size / 0x100000);
            *size /= (double)100;
            return L" Mb";
        }
        if (*size >= 0x400)
        {
            *size *= (double)100;
            *size = round(*size / 0x400);
            *size /= (double)100;
            return L" Kb";
        }
        return L" b";
    }

    hstring FileTabView::GetRealPath(hstring const& dirtyPath)
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

    void FileTabView::LoadPath(hstring path)
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
            path = GetRealPath(path); // get the real path with GetFinalPathNameByHandle, can return empty
            if (path == L"")
            {
                return;
            }

            InputModeButton().IsChecked(false);
            FileListView().Items().Clear();
            ProgressRing().Visibility(Visibility::Visible);

            WIN32_FIND_DATA data{};
            HANDLE handle = FindFirstFile((path + L"*").c_str(), &data);
            if (handle != INVALID_HANDLE_VALUE)
            {
                bool showSpecialFolders = ShowSpecialFolders();
                do
                {
                    hstring fileName = to_hstring(data.cFileName);
                    if (showSpecialFolders || (fileName != L".." && fileName != L"."))
                    {
                        WCHAR combinedPath[ALTERNATE_MAX_PATH]{ 0 };
                        PathCombine(combinedPath, path.c_str(), fileName.c_str());
                        hstring filePath = to_hstring(combinedPath);

                        int64_t size = (static_cast<int64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
                        DateTime modifiedDate = clock::from_FILETIME(data.ftLastWriteTime);
                        FileEntryView view = FileEntryView(fileName, filePath, size, data.dwFileAttributes);
                        view.LastWrite(modifiedDate);
                        view.Background(FileListView().Background());
                        FileListView().Items().Append(view);
                    }
                } while (FindNextFile(handle, &data));
                FindClose(handle);
                previousPath = hstring{ path };
            }
            else
            {
                MainWindow::Current().NotifyError(GetLastError());
            }

            m_propertyChanged(*this, PropertyChangedEventArgs{ L"ItemCount" });
            PathInputBox().Text(path);
            ProgressRing().Visibility(Visibility::Collapsed);
        }
        else
        {
            MainWindow::Current().NotifyUser(L"Path not found \"" + path + L"\"", InfoBarSeverity::Error);
        }
    }

    void FileTabView::SavePage()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        settings.Values().Insert(L"UseSearchRegex", box_value(UseRegexButton().IsChecked()));
    }

    void FileTabView::Search(hstring const& query, IVector<IInspectable> const& suggestions)
    {
        wregex re = wregex(query.data(), regex_constants::icase);
        IVector<IInspectable> listViewItems = FileListView().Items();
        
        for (IInspectable const& listViewItem : listViewItems)
        {
            FileEntryView entry = listViewItem.try_as<FileEntryView>();
            if (entry)
            {
                if (entry.FileName() != L"." && entry.FileName() != L".." && regex_search(entry.FileName().c_str(), re))
                {
                    suggestions.Append(box_value(entry.FileName()));
                }
            }
        }
    }

    inline bool FileTabView::ShowSpecialFolders()
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

    void FileTabView::SortFiles()
    {        
#define vect_type FileEntryView

        vector<vect_type> vect{};
        IVector<IInspectable> listViewItems = FileListView().Items();
        for (IInspectable const& listViewItem : listViewItems)
        {
            auto&& opt = listViewItem.try_as<FileEntryView>();
            if (opt)
            {
                vect.push_back(opt);
            }
        }
        FileListView().Items().Clear();

        if (vect.size() > 0)
        {
#ifdef _DEBUG
            Sorting::QuickSort<vect_type> sort{};
            try
            {
                sort.Sort(&vect, 0, vect.size() - 1, [](const FileEntryView& base, const FileEntryView& other)
                {
                    if (base.IsDirectory() && !other.IsDirectory())
                    {
                        return static_cast<int8_t>(-1);
                    }
                    if (!base.IsDirectory() && other.IsDirectory())
                    {
                        return static_cast<int8_t>(1);
                    }

                    if (base.FileBytes() > other.FileBytes())
                    {
                        return static_cast<int8_t>(-1);
                    }
                    if (base.FileBytes() < other.FileBytes())
                    {
                        return static_cast<int8_t>(1);
                    }
                    return static_cast<int8_t>(0);
                });
            }
            catch (hresult_error const& ex)
            {
                MainWindow::Current().NotifyError(ex.code(), L"Failed to sort files.");
            }
#else
            Sorting::QuickSort<vect_type> sort{};
            try
            {
                sort.Sort(&vect, 0, vect.size() - 1);
            }
            catch (hresult_error const& ex)
            {
                MainWindow::Current().NotifyError(ex.code(), L"Failed to sort files.");
            }
#endif // _DEBUG

        }

        for (size_t i = 0; i < vect.size(); i++)
        {
            listViewItems.Append(vect[i]);
        }

#undef vect_type
    }

    void FileTabView::MainWindow_Closed(IInspectable const&, WindowEventArgs const&)
    {
        SavePage();
    }
}
