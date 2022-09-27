#include "pch.h"
#include "FileTabView.xaml.h"
#if __has_include("FileTabView.g.cpp")
#include "FileTabView.g.cpp"
#endif

#include <chrono>
#include <regex>
#include "shlwapi.h"
#include "fileapi.h"
#include "DirectorySizeCalculator.h"
#include "QuickSort.h"
#include "Helper.h"
#include "FilePropertiesWindow.xaml.h"
#include "FileSearchWindow.xaml.h"
#include "FileInfo.h"
#include "DirectoryEnumerator.h"

using namespace std;

using namespace ::Winnerino;
using namespace ::Winnerino::Storage;

using namespace winrt;

using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::Windows::ApplicationModel::Resources;

using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Globalization::DateTimeFormatting;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Security::Cryptography;
using namespace winrt::Windows::Security::Cryptography::Core;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::System;
using namespace winrt::Windows::UI::Notifications; 


namespace winrt::Winnerino::implementation
{
    FileTabView::FileTabView()
    {
        InitializeComponent();

        windowClosedToken = MainWindow::Current().Closed({ this, &FileTabView::MainWindow_Closed });
        windowSizeChangedToken = MainWindow::Current().SizeChanged({ this, &FileTabView::Window_SizeChanged });
        loadingEventToken = Loading({ this, &FileTabView::UserControl_Loading });

        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        
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
    }

    FileTabView::FileTabView(const hstring& path) : FileTabView()
    {
        previousPath = path;
        InitializeComponent();
    }

    FileTabView::~FileTabView()
    {
        MainWindow::Current().Closed(windowClosedToken);
        MainWindow::Current().SizeChanged(windowSizeChangedToken);
    }


    void FileTabView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        SetLayout(MainWindow::Current().Size().Width);
        FilesListView().Focus(FocusState::Programmatic);
    }

    void FileTabView::UserControl_Loading(FrameworkElement const&, IInspectable const&)
    {
        Loading(loadingEventToken);

        if (!previousPath.empty())
        {
            LoadPath(previousPath);
        }
    }

    void FileTabView::PathInputBox_SuggestionChosen(AutoSuggestBox const& sender, AutoSuggestBoxSuggestionChosenEventArgs const& args)
    {
        std::string s = to_string(sender.Text());

        if (listingDrives || s.empty())
        {
            optional<hstring> opt = args.SelectedItem().try_as<hstring>();
            if (opt)
            {
                sender.Text(unbox_value<hstring>(args.SelectedItem()));
            }
            else
            {
                FrameworkElement element = args.SelectedItem().try_as<FrameworkElement>();
                if (element)
                {
                    sender.Text(element.FindName(L"PathTextBlock").as<TextBlock>().Text());
                }
            }
        }
        else if (!listingDrives)
        {
            for (size_t i = s.size() - 1; i > 0; i--)
            {
                if (s[i] == '\\')
                {
                    std::string substr = s.substr(0, i + 1);
                    sender.SetValue(AutoSuggestBox::TextProperty(), box_value(to_hstring(substr) + unbox_value<hstring>(args.SelectedItem())));
                    return;
                }
            }
        }
    }

    void FileTabView::PathInputBox_GotFocus(IInspectable const& sender, RoutedEventArgs const&)
    {
        AutoSuggestBox box = sender.as<AutoSuggestBox>();
        IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
        CompletePath(box.Text(), &suggestions);
        box.ItemsSource(suggestions);
    }

    void FileTabView::PathInputBox_TextChanged(AutoSuggestBox const& sender, AutoSuggestBoxTextChangedEventArgs const& args)
    {
        if (args.Reason() == AutoSuggestionBoxTextChangeReason::UserInput)
        {
            IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
            CompletePath(sender.Text(), &suggestions);
            sender.ItemsSource(suggestions);
        }
    }

    void FileTabView::PathInputBox_QuerySubmitted(AutoSuggestBox const& sender, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        if (!previousPath.empty())
        {
            backStack.push(previousPath);
        }

        if (gridViewLoaded)
        {
            LoadToGridView(args.QueryText());
        }
        else
        {
            LoadPath(args.QueryText());
        }
    }

    void FileTabView::FileListView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        e_propertyChanged(*this, PropertyChangedEventArgs{ L"SelectedItemsCount" });

        if (FilesListView().SelectedItems().Size() == 1)
        {
            FileEntryView view = FilesListView().SelectedItem().try_as<FileEntryView>();
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

            auto&& selectedItems = FilesListView().SelectedItems();
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
            hstring ext = format_size(&size);
            FileSizeTextBlock().Text(to_hstring(size) + ext);
        }
    }

    void FileTabView::BackButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        GoBack();
    }

    void FileTabView::ForwardButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        GoForward();
    }

    void FileTabView::UpButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        wstring work = previousPath.c_str();
        size_t i = work.size() - (work.ends_with('\\') ? 2 : 1);
        for (; i > 0; i--)
        {
            if (work[i] == '\\')
            {
                backStack.push(previousPath);

                if (listViewLoaded)
                {
                    LoadPath(hstring(work.substr(0, i)));
                }
                else
                {
                    LoadToGridView(hstring(work.substr(0, i)));
                }
                return;
            }
        }
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
    }

    void FileTabView::SearchButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto&& w = make<FileSearchWindow>();
        
        Rect windowBounds = MainWindow::Current().Bounds();
        windowBounds.X = MainWindow::Current().Position().X;
        windowBounds.Y = MainWindow::Current().Position().Y;

        w.SetPosition(
            PointInt32(static_cast<int32_t>(windowBounds.X + (windowBounds.Width / 2.0f) - (w.Bounds().Width / 2.0f)), 
            static_cast<int32_t>(windowBounds.Y + (windowBounds.Height / 2.0f) - (w.Bounds().Height / 2.0f)))
        );
        w.Activate();
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
        //if (!previousInput.empty())
        //{
        //    ApplicationDataContainer explorerSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        //    if (explorerSettings)
        //    {
        //        ApplicationDataContainer favorites = explorerSettings.Containers().TryLookup(L"Favorites");
        //        if (!favorites)
        //        {
        //            favorites = explorerSettings.CreateContainer(L"Favorites", ApplicationDataCreateDisposition::Always);
        //        }

        //        IPropertySet values = favorites.Values();
        //        for (auto&& value : values)
        //        {
        //            const optional<hstring>& optional = value.try_as<hstring>();
        //            if (optional && optional.Value() == previousPath)
        //            {
        //                return;
        //            }
        //        }

        //        HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
        //        hstring key = CryptographicBuffer::EncodeToHexString(
        //            provider.HashData(CryptographicBuffer::ConvertStringToBinary(previousPath, BinaryStringEncoding::Utf8)));
        //        favorites.Values().Insert(key, box_value(previousInput));
        //        // PathInputBox().Text()
        //    }
        //}
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

    IAsyncAction FileTabView::CutFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        DataPackage data{};
        data.RequestedOperation(DataPackageOperation::Move);
        IVector<IStorageItem> items{ single_threaded_vector<IStorageItem>() };

        IVector<IInspectable> selectedItems = FilesListView().SelectedItems();
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
            // I18N: Clipboard actions -> "Moved" singular and plural "No files selected to move"
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

        toastGenerator.ShowToastNotification(message);
    }

    IAsyncAction FileTabView::CopyFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        DataPackage data{};
        data.RequestedOperation(DataPackageOperation::Copy);
        IVector<IStorageItem> items{ single_threaded_vector<IStorageItem>() };

        IVector<IInspectable> selectedItems = FilesListView().SelectedItems();
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

        if (items.Size() > 0)
        {
            //I18N: Clipboard actions -> "Copied" singular and plural "No files selected to copy"
            hstring message{};
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

            toastGenerator.ShowToastNotification(message);
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

    IAsyncAction FileTabView::DeleteFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        IVector<IInspectable> selectedItems = nullptr;
        if (listViewLoaded)
        {
            selectedItems = FilesListView().SelectedItems();
        }
        else
        {
            selectedItems = FilesGridView().SelectedItems();
        }

        if (selectedItems.Size() == 1)
        {
            FileEntryView view = FilesListView().SelectedItem().try_as<FileEntryView>();
            try
            {
                if (view)
                {
                    view.Delete();
                }
            }
            catch (const std::exception& ex)
            {
                OutputDebugString(to_hstring(ex.what()).c_str());
                MainWindow::Current().NotifyUser(L"Could not delete " + view.FileName(), InfoBarSeverity::Warning);
            }
        }
        else if (selectedItems.Size() > 0)
        {
            short state = GetKeyState(static_cast<int>(VirtualKey::Shift));
            IVector<FileEntryView> selectedFileEntries{ single_threaded_vector<FileEntryView>() };

            // Copy collection to avoid iterator corruption later
            for (size_t i = 0; i < selectedItems.Size(); i++)
            {
                auto&& view = selectedItems.GetAt(i).as<FileEntryView>();
                if (view)
                {
                    selectedFileEntries.Append(view);
                }
            }

            if (state & 0x8000)
            {
                for (IInspectable inspectable : selectedFileEntries)
                {
                    FileEntryView view = inspectable.try_as<FileEntryView>();

                    try
                    {
                        view.Delete();
                    }
                    catch (const std::exception& ex)
                    {
                        OutputDebugString(to_hstring(ex.what()).c_str());
                        MainWindow::Current().NotifyUser(L"Could not delete " + view.FileName(), InfoBarSeverity::Warning);
                    }
                }
            }
            else
            {
                for (FileEntryView view : selectedFileEntries)
                {
                    Grid grid{};
                    grid.ColumnSpacing(15);

                    ColumnDefinition colDef{};
                    colDef.Width(GridLength(2, GridUnitType::Star));
                    grid.ColumnDefinitions().Append(colDef);

                    colDef = ColumnDefinition();
                    colDef.Width(GridLength(1, GridUnitType::Star));
                    grid.ColumnDefinitions().Append(colDef);

                    TextBlock name{};
                    TextBlock path{};
                    path.Name(L"FilePath");
                    name.TextTrimming(TextTrimming::CharacterEllipsis);
                    path.TextWrapping(TextWrapping::Wrap);

                    name.Text(view.FileName());
                    path.Text(view.FilePath());

                    Grid::SetColumn(name, 1);
                    Grid::SetColumn(path, 0);
                    grid.Children().Append(name);
                    grid.Children().Append(path);

                    DeleteMultipleListView().Items().Append(grid);
                }

                //DeleteMultipleContentDialog().Content().as<Grid>().MinWidth(ActualWidth() * 0.6);
                auto&& dialogResult = co_await DeleteMultipleContentDialog().ShowAsync();
                if (dialogResult == ContentDialogResult::Primary)
                {
                    DispatcherQueue().TryEnqueue([this]()
                    {
                        auto&& toDelete = DeleteMultipleListView().SelectedItems();
                        auto&& selectedItems = FilesListView().SelectedItems();

                        for (auto&& item : toDelete)
                        {
                            UIElementCollection children = item.as<Grid>().Children();
                            if (children)
                            {
                                IInspectable child = children.GetAt(1);
                                hstring path = child.as<TextBlock>().Text();
                                hstring name = child.as<TextBlock>().Name();

                                for (IInspectable inspectable : selectedItems)
                                {
                                    FileEntryView view = inspectable.try_as<FileEntryView>();
                                    if (view && view.FilePath() == path)
                                    {
                                        try
                                        {
                                            view.Delete();
                                        }
                                        catch (std::exception const& ex)
                                        {
                                            OutputDebugString((to_hstring(ex.what()) + L"\n").c_str());
                                        }
                                        // breaking, we should only have 1 file with the same path
                                        break;
                                    }
                                }
                            }
                        }
                    });
                }
            }
        }
    }

    void FileTabView::OpenWithFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void FileTabView::CopyPathFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (listViewLoaded)
        {
            IVector<IInspectable> selectedItems = FilesListView().SelectedItems();
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
        else if (gridViewLoaded)
        {
            IVector<IInspectable> selectedItems = FilesListView().SelectedItems();
            for (IInspectable item : selectedItems)
            {
                FileLargeView view = item.try_as<FileLargeView>();
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
    }

    IAsyncAction FileTabView::OpenInExplorerFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (listViewLoaded)
        {
            IVector<IInspectable> selectedItems = FilesListView().SelectedItems();
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
        else if (gridViewLoaded)
        {
            IVector<IInspectable> selectedItems = FilesGridView().SelectedItems();
            for (IInspectable item : selectedItems)
            {
                FileLargeView view = item.try_as<FileLargeView>();
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
    }

    void FileTabView::ShowPropertiesFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Window window = make<FilePropertiesWindow>();
        window.Activate();
    }

    IAsyncAction FileTabView::SpotlightImporter_Click(IInspectable const&, RoutedEventArgs const&)
    {
        co_await SpotlightImporterDialog().ShowAsync();
    }

    void FileTabView::PathInputBox_PreviewKeyDown(IInspectable const&, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == VirtualKey::Right)
        {
            IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
            if (!PathInputBox().Text().ends_with(L"\\"))
            {
                PathInputBox().Text(PathInputBox().Text() + L"\\");
            }
            CompletePath(PathInputBox().Text(), &suggestions);
            PathInputBox().ItemsSource(suggestions);

            previousPath = PathInputBox().Text();
        }
    }

    void FileTabView::ListViewFlyout_Opening(IInspectable const&, IInspectable const&)
    {
        //TODO: get the "Opens with" list of programs + customs
#ifdef DEBUG
        if (FilesListView().SelectedItems().Size() == 1)
        {
            FileEntryView view = FilesListView().SelectedItem().try_as<FileEntryView>();
            if (view)
            {
                hstring ext = view.FileExtension() + L"\\OpenWithProgids";

                HKEY hKey{};
                if (SUCCEEDED(RegOpenKeyEx(HKEY_CLASSES_ROOT, ext.c_str(), 0, KEY_READ, &hKey)))
                {
                    DWORD cchClassName = MAX_PATH;
                    DWORD subkeyCount = 0;
                    DWORD maxSubkeySize = 0;
                    DWORD maxClassNameLength = 0;
                    DWORD valueCount = 0;
                    DWORD maxValueNameLength = 0;
                    DWORD maxValueLength = 0;
                    DWORD securityDesc = 0;
                    WCHAR className[MAX_PATH];
                    FILETIME keyLastWrite{};

                    LSTATUS resCode = RegQueryInfoKey(
                        hKey,                 // key handle
                        className,            // buffer for class name 
                        &cchClassName,        // size of class string
                        NULL,                 // reserved 
                        &subkeyCount,         // number of subkeys
                        &maxSubkeySize,       // longest subkey size 
                        &maxClassNameLength,  // longest class string
                        &valueCount,          // number of values for this key
                        &maxValueNameLength,  // longest value name 
                        &maxValueLength,      // longest value data 
                        &securityDesc,        // longest value data 
                        &keyLastWrite         // last write time 
                    );

                    if (SUCCEEDED(resCode))
                    {
                        for (size_t i = 0; i < subkeyCount; i++)
                        {
                            DWORD cbName = MAX_PATH;
                            WCHAR subkeyName[MAX_PATH];
                            resCode = RegEnumKeyEx(
                                hKey,
                                i,
                                subkeyName,
                                &cbName,
                                NULL,
                                NULL,
                                NULL,
                                &keyLastWrite
                            );

                            if (FAILED(resCode))
                            {
                                __debugbreak();
                            }
                        }
                    }
                }
                else
                {
                    __debugbreak();
                }
            }
        }
#endif // DEBUG

    }

    void FileTabView::ListViewButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        GridViewButton().IsChecked(false);

        if (FilesGridView().Visibility() == Visibility::Visible && GridViewScrollViewer().Visibility() == Visibility::Visible)
        {
            FilesGridView().Visibility(Visibility::Collapsed);
            GridViewScrollViewer().Visibility(Visibility::Collapsed);
        }

        // show files default
        if (FilesListView().Visibility() == Visibility::Collapsed)
        {
            FilesListView().Visibility(Visibility::Visible);
        }

        if (!listViewLoaded)
        {
            LoadPath(previousPath);
        }
    }

    IAsyncAction FileTabView::GridViewButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ListViewButton().IsChecked(false);

        // Show files as thumbnails
        if (FilesListView().Visibility() == Visibility::Visible)
        {
            FilesListView().Visibility(Visibility::Collapsed);
        }

        if (FilesGridView().Visibility() == Visibility::Collapsed && GridViewScrollViewer().Visibility() == Visibility::Collapsed)
        {
            FilesGridView().Visibility(Visibility::Visible);
            GridViewScrollViewer().Visibility(Visibility::Visible);
        }

        if (!gridViewLoaded)
        {
            co_await LoadToGridView(previousPath);
        }
    }

    IAsyncAction FileTabView::ListView_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
    {
        FileEntryView entry = FilesListView().SelectedItem().try_as<FileEntryView>();
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
                co_await Open(entry.FilePath());
            }
        }
    }

    IAsyncAction FileTabView::FilesGridView_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
    {
        FileLargeView view = FilesGridView().SelectedItem().try_as<FileLargeView>();
        if (view)
        {
            if (view.IsDirectory())
            {
                if (!previousPath.empty())
                {
                    backStack.push(previousPath);
                }

                co_await LoadToGridView(view.FilePath());
            }
            else
            {
                co_await Open(view.FilePath());
            }
        }
        co_return;
    }

    void FileTabView::UserControl_PointerPressed(IInspectable const&, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        auto point = e.GetCurrentPoint(ContentGrid());
        if (point.Properties().IsXButton1Pressed()) // Go forward
        {
            GoBack();
        }
        else if (point.Properties().IsXButton2Pressed()) // Go back
        {
            GoForward();
        }
    }


    void FileTabView::CompletePath(hstring const& query, IVector<IInspectable>* const& suggestions)
    {
        DirectoryEnumerator enumerator{ false };

        if (query.size() > 2)
        {
            wstring path = query.data();
            wstring name{};
            if (!path.ends_with('\\'))
            {
                for (int i = static_cast<int>(path.length()) - 1; i >= 0; i--)
                {
                    if (path[i] == '\\')
                    {
                        name = path.substr(i + 1, path.length() - i);
                        path = path.substr(0, i + 1);
                        break;
                    }
                }
            }

            unique_ptr<vector<FileInfo>> dirs{ enumerator.GetFolders(hstring(path)) };
            if (dirs.get())
            {
                wregex searchQuery{ name, regex_constants::icase | regex_constants::ECMAScript };
                for (FileInfo info : *dirs)
                {
                    if (name.empty() || regex_search(info.Path().c_str(), searchQuery))
                    {
                        suggestions->Append(box_value(info.Name()));
                    }
                }
            }

            listingDrives = false;
        }
        else
        {
            /*ApplicationDataContainer explorerSettings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
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

                            suggestions->Append(grid);
                        }
                    }
                }
            }*/

            unique_ptr<vector<hstring>> drives{ enumerator.EnumerateDrives() };
            for (size_t i = 0; i < drives->size(); i++)
            {
                if (PathFileExists(drives->at(i).c_str()))
                {
                    suggestions->Append(box_value(drives->at(i)));
                }
            }
            listingDrives = true;

            IPropertySet settings = get_or_create_container(L"Explorer").Values();
            if (unbox_value_or(settings.TryLookup(L"ListLibraries"), true))
            {
                // Get I18N library names
                ResourceLoader resLoader{};

                suggestions->Append(box_value(resLoader.GetString(L"FolderNameDesktop")));
                suggestions->Append(box_value(resLoader.GetString(L"FolderNameDownloads")));
                suggestions->Append(box_value(resLoader.GetString(L"FolderNameDocuments")));
                suggestions->Append(box_value(resLoader.GetString(L"FolderNameMusic")));
                suggestions->Append(box_value(resLoader.GetString(L"FolderNamePictures")));
                suggestions->Append(box_value(resLoader.GetString(L"FolderNameVideos")));
            }
        }
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
        ProgressVisibility(true);

        listViewLoaded = true;

        if (PathFileExists(path.c_str()))
        {
            path = GetRealPath(path); // get the real path with GetFinalPathNameByHandle, can return empty
            if (path.empty())
            {
                return;
            }

            _files.Clear();
            //FilesGridView().Items().Clear();
            PathInputBox().Text(path);
            fileEntryDeletedRevokers.clear();

            //I18N: Number inter
            HiddenFilesCount().Text(L"0");

            concurrency::create_task([this, _path = path]()
            {
                WIN32_FIND_DATA data{};
                HANDLE handle = FindFirstFile((_path + L"*").c_str(), &data);
                if (handle != INVALID_HANDLE_VALUE)
                {
                    bool showSpecialFolders = false;
                    bool hideSystemFiles = false;
                    GetOptions(&showSpecialFolders, &hideSystemFiles);

                    uint32_t loadedFiles = 0;
                    uint32_t hiddenFiles = 0;

                    do
                    {
                        const wchar_t dot[2]{ '.', '\0' };
                        const wchar_t dotdot[3]{ '.', '.', '\0' };
                        if (showSpecialFolders || (wcscmp(data.cFileName, dot) != 0 && wcscmp(data.cFileName, dotdot) != 0))
                        {
                            FileInfo fileInfo(&data, _path);
                            if (fileInfo.IsSystemFile() && hideSystemFiles)
                            {
                                hiddenFiles++;
                                continue;
                            }

                            DispatcherQueue().TryEnqueue([this, cfileInfo = move(fileInfo)]()
                            {
                                FilesListView().ItemsSource(nullptr);

                                FileEntryView view{ cfileInfo.Name(), cfileInfo.Path(), cfileInfo.Size(), cfileInfo.Attributes() };
                                view.LastWrite(cfileInfo.LastWrite());
                                _files.Append(view);

                                fileEntryDeletedRevokers.push_back(
                                    view.Deleted(auto_revoke, [&](Winnerino::FileEntryView const& sender, IInspectable const&)
                                    {
                                        uint32_t index;
                                        if (_files.IndexOf(sender, index))
                                        {
                                            _files.RemoveAt(index);
                                        }
                                    })
                                );

                                FilesListView().ItemsSource(_files);
                            });

                            loadedFiles++;
                        }
                    } while (FindNextFile(handle, &data));
                    FindClose(handle);

                    previousPath = hstring(_path);
                    
                    if (loadedFiles > 5000)
                    {
                        MainWindow::Current().NotifyUser(L"Sorting operations may fail because of the large number of files loaded", InfoBarSeverity::Warning);
                    }

                    DispatcherQueue().TryEnqueue([&, hiddenFiles]()
                    {
                        HiddenFilesCount().Text(to_hstring(hiddenFiles));
                    });
                }
                else
                {
                    MainWindow::Current().NotifyError(GetLastError());
                }

                DispatcherQueue().TryEnqueue([this]()
                {
                    e_propertyChanged(*this, PropertyChangedEventArgs(L"ItemCount"));
                    ProgressVisibility(false);
                });
            });
        }
        else
        {
            MainWindow::Current().NotifyUser(L"Path not found \"" + path + L"\"", InfoBarSeverity::Error);
        }
    }

    IAsyncAction FileTabView::LoadToGridView(hstring path)
    {
        ProgressVisibility(true);
        
        gridViewLoaded = true;

        if (PathFileExists(path.c_str()))
        {
            path = GetRealPath(path);
            previousPath = hstring(path);

            //_files.Clear();
            FilesGridView().Items().Clear();
            PathInputBox().Text(path);
            fileEntryDeletedRevokers.clear();

            //I18N: Number inter
            HiddenFilesCount().Text(L"0");

            // List files
            DirectoryEnumerator enumerator{};
            unique_ptr<vector<FileInfo>> vect(enumerator.GetEntries(path));

            ProgressVisibility(false);

            if (vect.get())
            {
                vector<IAsyncOperation<bool>> tasks{};

                for (size_t i = 0; i < vect->size(); i++)
                {
                    FileLargeView view{};
                    FilesGridView().Items().Append(view);

                    try
                    {
                        co_await view.Initialize(vect->at(i).Path(), !vect->at(i).IsDirectory());
                    }
                    catch (hresult_error const&)
                    {
                        DispatcherQueue().TryEnqueue([this, /*path = vect->at(i).Path()*/ view]
                        {
                            try
                            {
                                uint32_t index;
                                if (FilesGridView().Items().IndexOf(view, index))
                                {
                                    FilesGridView().Items().RemoveAt(index);
                                }
                            }
                            catch (hresult_error const&)
                            {
                                OutputDebugString(L"\n");
                            }
                        });
                    }
                }
            }
        }
    }

    inline void FileTabView::GetOptions(bool* showSpecialFolders, bool* hideSystemFiles)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        if (settings)
        {
            *showSpecialFolders = unbox_value_or(settings.Values().TryLookup(L"ShowSpecialFolders"), false);
            *hideSystemFiles = unbox_value_or(settings.Values().TryLookup(L"HideSystemFiles"), false);
        }
    }

    void FileTabView::SavePage()
    {
    }

    void FileTabView::Search(hstring const& query, IVector<IInspectable> const& suggestions)
    {
        try
        {
            wregex re = wregex(query.data(), regex_constants::icase);
            IVector<IInspectable> listViewItems = FilesListView().Items();

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
        catch (regex_error const& ex)
        {
            MainWindow::Current().NotifyError(ex.code(), L"Cannot create search regex");
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
        // TODO: Use VisualManager to set layout properties

        if (width < 730 && CommandBarColumn().Width().GridUnitType != GridUnitType::Pixel)
        {
            CommandBarColumn().Width(GridLengthHelper::FromPixels(0));
        }
        else if (CommandBarColumn().Width().GridUnitType != GridUnitType::Auto)
        {
            CommandBarColumn().Width(GridLengthHelper::Auto());
        }


        if (width < 500 && InputModeButtonColumn().Width().Value != 0)
        {
            InputModeButtonColumn().Width(GridLengthHelper::FromPixels(0));
        }
        else if (InputModeButtonColumn().Width().Value != 40)
        {
            InputModeButtonColumn().Width(GridLengthHelper::FromPixels(40));
        }
    }

    void FileTabView::GoForward()
    {
        if (!forwardStack.empty())
        {
            hstring path = forwardStack.top();
            forwardStack.pop();
            backStack.push(path);

            if (listViewLoaded)
            {
                LoadPath(path);
            }
            else
            {
                LoadToGridView(path);
            }
        }
    }

    void FileTabView::GoBack()
    {
        if (!backStack.empty())
        {
            hstring path = backStack.top();
            backStack.pop();
            forwardStack.push(previousPath);

            if (listViewLoaded)
            {
                LoadPath(path);
            }
            else
            {
                LoadToGridView(path);
            }
        }
    }

    IAsyncAction FileTabView::Open(hstring const& filePath)
    {
        // TODO: Support when the file doesn't have a registered default app
        try
        {
            StorageFile file = co_await StorageFile::GetFileFromPathAsync(filePath);
            if (!co_await Launcher::LaunchFileAsync(file))
            {
                // I18N
                MainWindow::Current().NotifyUser(L"Failed to open file.", InfoBarSeverity::Warning);
            }
        }
        catch (const hresult& ex)
        {
            MainWindow::Current().NotifyError(ex);
        }
    }

    void FileTabView::ProgressVisibility(bool const& value)
    {
        _progressVisibility = value;
        e_propertyChanged(*this, PropertyChangedEventArgs(L"ProgressVisibility"));
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
