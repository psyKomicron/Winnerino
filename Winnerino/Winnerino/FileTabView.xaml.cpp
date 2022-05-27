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
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Globalization::DateTimeFormatting;
using namespace winrt::Windows::Security::Cryptography;
using namespace winrt::Windows::Security::Cryptography::Core;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Notifications;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileTabView::FileTabView()
    {
        InitializeComponent();
        
        windowClosedToken = MainWindow::Current().Closed({ this, &FileTabView::MainWindow_Closed });
        windowSizeChangedToken = MainWindow::Current().SizeChanged({ this, &FileTabView::Window_SizeChanged });
    }

    FileTabView::FileTabView(const hstring& path) : FileTabView()
    {
        InitializeComponent();
        LoadPath(path);
    }

    FileTabView::~FileTabView()
    {
        MainWindow::Current().Closed(windowClosedToken);
        MainWindow::Current().SizeChanged(windowSizeChangedToken);
    }


    void FileTabView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        std::optional<bool> isChecked = settings.Values().Lookup(L"UseSearchRegex").try_as<bool>();
        UseRegexButton().IsChecked(isChecked.value_or(false));
        ContentNavigationView().IsPaneOpen(unbox_value_or<bool>(settings.Values().TryLookup(L"IsPaneOpen"), true));

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

        SetLayout(MainWindow::Current().Size().Width);
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
            for (size_t i = s.size() - 1; i > 0; i--)
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

    void FileTabView::FileListView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"SelectedItemsCount" });

        if (FileListView().SelectedItems().Size() == 1)
        {
            FileEntryView view = FileListView().SelectedItem().try_as<FileEntryView>();
            if (view)
            {
                FileOpenAppTextBlock().Text(view.OpensWith());
                FileSizeTextBlock().Text(view.FileSize());
                FilePerceivedTypeTextBlock().Text(view.PerceivedType());
                FileLastWriteTextBlock().Text(formatter.Format(view.LastWrite()));
                FileDangerousFontIcon().Visibility(view.IsFileDangerous() ? Visibility::Visible : Visibility::Collapsed);
            }
        }
        else
        {
            FileOpenAppTextBlock().Text(L"");
            FilePerceivedTypeTextBlock().Text(L"");
            FileLastWriteTextBlock().Text(L"");
            FileDangerousFontIcon().Visibility(Visibility::Collapsed);

            auto&& selectedItems = FileListView().SelectedItems();
            uint64_t combinedSize = 0;
            for (IInspectable selectedItem : selectedItems)
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

    void FileTabView::BackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!backStack.empty())
        {
            hstring path = backStack.top();
            backStack.pop();
            forwardStack.push(previousPath);
            LoadPath(path);
        }
    }

    void FileTabView::ForwardButton_Click(IInspectable const&, RoutedEventArgs const&)
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

    IAsyncAction FileTabView::RenameAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
    }

    void FileTabView::DeleteAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
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
        SortFiles([](const FileEntryView& base, const FileEntryView& other)
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

    void FileTabView::SettingsButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().NavigateTo(xaml_typename<SettingsPage>());
    }

    void FileTabView::UpButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void FileTabView::ContentNavigationView_ItemInvoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& args)
    {
        MainWindow::Current().NavigateTo(xaml_typename<ExplorerPage>(), args.InvokedItem());
    }

    IAsyncAction FileTabView::CutFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        DataPackage data{};
        data.RequestedOperation(DataPackageOperation::Move);
        IVector<IStorageItem> items{ single_threaded_vector<IStorageItem>() };

        IVector<IInspectable> selectedItems = FileListView().SelectedItems();
        for (IInspectable selectedItem : selectedItems)
        {
            FileEntryView view = selectedItem.try_as<FileEntryView>();
            if (view)
            {
                if (view.IsDirectory())
                {
                    items.Append(co_await StorageFolder::GetFolderFromPathAsync(view.FilePath()));
                }
                else
                {
                    items.Append(co_await StorageFile::GetFileFromPathAsync(view.FilePath()));
                }
            }
        }

        hstring message{};
        if (items.Size() > 0)
        {
            data.SetStorageItems(items);
            Clipboard::SetContent(data);
            if (items.Size() == 1)
            {
                message = L"Moved \"" + items.GetAt(0).Name() + L"\" to clipboard";
            }
            else
            {
                message = L"Moved " + to_hstring(items.Size()) + L" to clipboard";
            }
        }
        else
        {
            message = L"No files selected to move";
        }

        if (unbox_value_or<bool>(ApplicationData::Current().LocalSettings().Values().TryLookup(L"NotificationsEnabled"), true))
        {
            XmlDocument toastContent{};
            XmlElement root = toastContent.CreateElement(L"toast");
            toastContent.AppendChild(root);

            XmlElement visual = toastContent.CreateElement(L"visual");
            root.AppendChild(visual);

            XmlElement binding = toastContent.CreateElement(L"binding");
            binding.SetAttribute(L"template", L"ToastText01");
            visual.AppendChild(binding);

            XmlElement text = toastContent.CreateElement(L"text");
            text.SetAttribute(L"id", L"1");
            text.InnerText(message);
            binding.AppendChild(text);

            ToastNotification toastNotif{ toastContent };
            ToastNotificationManager::CreateToastNotifier().Show(toastNotif);
        }
    }

    IAsyncAction FileTabView::CopyFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // I18N

        DataPackage data{};
        data.RequestedOperation(DataPackageOperation::Copy);
        IVector<IStorageItem> items{ single_threaded_vector<IStorageItem>() };

        IVector<IInspectable> selectedItems = FileListView().SelectedItems();
        for (IInspectable selectedItem : selectedItems)
        {
            FileEntryView view = selectedItem.try_as<FileEntryView>();
            if (view)
            {
                if (view.IsDirectory())
                {
                    items.Append(co_await StorageFolder::GetFolderFromPathAsync(view.FilePath()));
                }
                else
                {
                    items.Append(co_await StorageFile::GetFileFromPathAsync(view.FilePath()));
                }
            }
        }

        hstring message{};
        if (items.Size() > 0)
        {
            data.SetStorageItems(items);
            Clipboard::SetContent(data);
            
            if (items.Size() == 1)
            {
                message = L"Copied \"" + items.GetAt(0).Name() + L"\" to clipboard";
            }
            else
            {
                message = L"Copied " + to_hstring(items.Size()) + L" to clipboard";
            }
        }
        else
        {
            message = L"No files selected to copy";
        }

        if (unbox_value_or<bool>(ApplicationData::Current().LocalSettings().Values().TryLookup(L"NotificationsEnabled"), true))
        {
            XmlDocument toastContent{};
            XmlElement root = toastContent.CreateElement(L"toast");
            toastContent.AppendChild(root);

            XmlElement visual = toastContent.CreateElement(L"visual");
            root.AppendChild(visual);

            XmlElement binding = toastContent.CreateElement(L"binding");
            binding.SetAttribute(L"template", L"ToastText01");
            visual.AppendChild(binding);

            XmlElement text = toastContent.CreateElement(L"text");
            text.SetAttribute(L"id", L"1");
            text.InnerText(message);
            binding.AppendChild(text);

            ToastNotification toastNotif{ toastContent };
            ToastNotificationManager::CreateToastNotifier().Show(toastNotif);
        }
    }

    IAsyncAction FileTabView::RenameFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        /*RenameContentDialog().Title(box_value(L"Rename " + _fileName));
        RenameTextBox().Text(_fileName);

        if (!_isDirectory)
        {
            PWSTR ext = PathFindExtension(_filePath.c_str());
            size_t extLength = wcslen(ext);
            size_t filePathLength = _fileName.size();
            int32_t selectionLength = static_cast<int32_t>(filePathLength) - static_cast<int32_t>(extLength);

            RenameTextBox().Select(0, selectionLength);
        }

        ContentDialogResult result = co_await RenameContentDialog().ShowAsync();
        if (result == ContentDialogResult::Primary)
        {
            if (!RenameTextBox().Text().empty())
            {
                RenameFile(RenameTextBox().Text(), GenerateUniqueNameCheckBox().IsChecked().GetBoolean());
            }
            else
            {
                MainWindow::Current().NotifyUser(L"Cannot rename file", InfoBarSeverity::Error);
            }
        }*/
        co_return;
    }

    void FileTabView::DeleteFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void FileTabView::OpenWithFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void FileTabView::MenuFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        IVector<IInspectable> selectedItems = FileListView().SelectedItems();
        for (IInspectable item : selectedItems)
        {
            FileEntryView view = item.try_as<FileEntryView>();
            if (view)
            {
                hstring filePath = view.FilePath();
                try
                {
                    DataPackage dataPackage{};
                    dataPackage.SetText(filePath);
                    Clipboard::SetContent(dataPackage);
                    MainWindow::Current().NotifyUser(L"Path copied to clipboard.", InfoBarSeverity::Success);
                }
                catch (const hresult_error& ex)
                {
                    MainWindow::Current().NotifyError(ex.code(), L"Failed to open file explorer");
                }
            }
        }
    }

    IAsyncAction FileTabView::OpenInExplorerFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        IVector<IInspectable> selectedItems = FileListView().SelectedItems();
        for (IInspectable item : selectedItems)
        {
            FileEntryView view = item.try_as<FileEntryView>();
            if (view)
            {
                hstring filePath = view.FilePath();
                hstring fileName = view.FileName();
                try
                {
                    string path = to_string(filePath);
                    string parent = path.substr(0, filePath.size() - fileName.size());
                    co_await Launcher::LaunchUriAsync(Uri{ to_hstring(parent) });
                }
                catch (const hresult_error& ex)
                {
                    MainWindow::Current().NotifyError(ex.code(), L"Failed to open file explorer");
                }
            }
        }
    }

    void FileTabView::FileSizeFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
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

    inline hstring FileTabView::FormatFileSize(double* size) const
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
            //FileListView().Items().Clear();
            _files.Clear();
            ProgressRing().Visibility(Visibility::Visible);

            concurrency::create_task([this, _path = path]()
            {
                WIN32_FIND_DATA data{};
                HANDLE handle = FindFirstFile((_path + L"*").c_str(), &data);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    bool showSpecialFolders = ShowSpecialFolders();
                    uint32_t loadedFiles = 0;
                    do
                    {
                        hstring fileName = to_hstring(data.cFileName);
                        if (showSpecialFolders || (fileName != L".." && fileName != L"."))
                        {
                            WCHAR combinedPath[ALTERNATE_MAX_PATH]{ 0 };
                            PathCombine(combinedPath, _path.c_str(), fileName.c_str());
                            hstring filePath = to_hstring(combinedPath);

                            int64_t size = (static_cast<int64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
                            DateTime modifiedDate = clock::from_FILETIME(data.ftLastWriteTime);

#define STA 1
#if STA
                            DispatcherQueue().TryEnqueue([this, fileName, filePath, size, attributes = data.dwFileAttributes, modifiedDate]()
                            {
                                FileEntryView view{ fileName, filePath, size, attributes };
                                view.LastWrite(modifiedDate);
                                view.Background(FileListView().Background());
                                _files.Append(view);
                            });
#endif // MTA


#if !STA
                            FileEntryView view{ fileName, filePath, size, data.dwFileAttributes };
                            view.LastWrite(modifiedDate);
                            _files.Append(view);
#endif // MTA
#undef STA

                            loadedFiles++;
                        }
                    } while (FindNextFile(handle, &data));
                    FindClose(handle);
                    previousPath = hstring{ _path };
                    
                    if (loadedFiles > 5000)
                    {
                        MainWindow::Current().NotifyUser(L"Sorting operations may fail because of the large number of files loaded", InfoBarSeverity::Warning);
                    }
                }
                else
                {
                    MainWindow::Current().NotifyError(GetLastError());
                }

                DispatcherQueue().TryEnqueue([this, _path]()
                {
                    PathInputBox().Text(_path);
                    m_propertyChanged(*this, PropertyChangedEventArgs{ L"ItemCount" });
                    ProgressRing().Visibility(Visibility::Collapsed);
                });
            });
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
        settings.Values().Insert(L"IsPaneOpen", box_value(ContentNavigationView().IsPaneOpen()));
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

    void FileTabView::SortFiles(const function<uint8_t(const FileEntryView&, const FileEntryView&)>& comparer)
    {        
        vector<FileEntryView> vect{};
        for (IInspectable const& listViewItem : _files)
        {
            auto&& opt = listViewItem.try_as<FileEntryView>();
            if (opt)
            {
                vect.push_back(opt);
            }
        }
        _files.Clear();

        if (vect.size() > 0)
        {
            Sorting::QuickSort<FileEntryView> sort{};
            try
            {
                sort.Sort(&vect, 0, vect.size() - 1, comparer);
            }
            catch (hresult_error const& ex)
            {
                MainWindow::Current().NotifyError(ex.code(), L"Failed to sort files.");
            }
        }
        
        for (size_t i = 0; i < vect.size(); i++)
        {
            _files.Append(vect[i]);
        }
    }

    void FileTabView::SetLayout(float const& width)
    {
        if (width < 730)
        {
            if (CommandBarColumn().Width().GridUnitType != GridUnitType::Pixel)
            {
                GridLength gridLength{};
                gridLength.Value = 0;
                gridLength.GridUnitType = GridUnitType::Pixel;
                CommandBarColumn().Width(gridLength);
            }
        }
        else
        {
            if (CommandBarColumn().Width().GridUnitType != GridUnitType::Auto)
            {
                GridLength gridLength{};
                gridLength.GridUnitType = GridUnitType::Auto;
                CommandBarColumn().Width(gridLength);
            }
        }

        if (width < 500)
        {
            if (InputModeButtonColumn().Width().GridUnitType != GridUnitType::Pixel)
            {
                GridLength gridLength{};
                gridLength.Value = 0;
                gridLength.GridUnitType = GridUnitType::Pixel;
                InputModeButtonColumn().Width(gridLength);
            }
        }
        else
        {
            if (InputModeButtonColumn().Width().GridUnitType != GridUnitType::Auto)
            {
                GridLength gridLength{};
                gridLength.GridUnitType = GridUnitType::Auto;
                InputModeButtonColumn().Width(gridLength);
            }
        }
    }

    void FileTabView::Window_SizeChanged(IInspectable const&, WindowSizeChangedEventArgs const& args)
    {
        SetLayout(args.Size().Width);
    }

    void FileTabView::MainWindow_Closed(IInspectable const&, WindowEventArgs const&)
    {
        SavePage();
    }
}