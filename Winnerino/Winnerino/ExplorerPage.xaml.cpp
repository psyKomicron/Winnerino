#include "pch.h"
#include "ExplorerPage.xaml.h"
#if __has_include("ExplorerPage.g.cpp")
#include "ExplorerPage.g.cpp"
#include "shlwapi.h"
#include "fileapi.h"
#endif
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation::Collections;

using namespace std;

using namespace winrt;

using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

using namespace Windows::Foundation;
using namespace Windows::Storage;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    ExplorerPage::ExplorerPage()
    {
        InitializeComponent();
    }

    void ExplorerPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        // get last path
        ApplicationDataContainer container = ApplicationData::Current().LocalSettings();
        IInspectable inspectable = container.Values().TryLookup(L"ExplorerLastPath");
        loadPath(unbox_value_or<hstring>(inspectable, L"C:\\"));

        inspectable = container.Values().TryLookup(L"ExplorerRecents");
        IVector<hstring> recents = unbox_value_or<IVector<hstring>>(inspectable, nullptr);
        if (recents)
        {
            for (hstring recent : recents)
            {
                if (recent != L"")
                {
                    MenuFlyoutItem item{};
                    item.Text(recent);
                    recentsMenuFlyout().Items().Append(item);
                }
            }
        }
    }

    void ExplorerPage::AutoSuggestBox_QuerySubmitted(AutoSuggestBox const&, AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        loadPath(args.QueryText());
    }

    void ExplorerPage::AutoSuggestBox_TextChanged(AutoSuggestBox const& sender, AutoSuggestBoxTextChangedEventArgs const& args)
    {
        if (args.Reason() == AutoSuggestionBoxTextChangeReason::UserInput)
        {
            IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
            completePath(sender.Text(), suggestions);
            sender.ItemsSource(suggestions);
        }
    }

    void ExplorerPage::AutoSuggestBox_GotFocus(IInspectable const& sender, RoutedEventArgs const&)
    {
        AutoSuggestBox box = sender.as<AutoSuggestBox>();
        IVector<IInspectable> suggestions{ single_threaded_vector<IInspectable>() };
        completePath(box.Text(), suggestions);
        box.ItemsSource(suggestions);
    }

    void ExplorerPage::AutoSuggestBox_SuggestionChosen(AutoSuggestBox const& sender, AutoSuggestBoxSuggestionChosenEventArgs const& args)
    {
        std::string s = to_string(sender.Text());
        for (int i = s.size() - 1; i >= 0; i--)
        {
            if (s[i] == '\\')
            {
                auto substr = s.substr(0, i + 1);
                sender.Text(to_hstring(substr) + unbox_value<hstring>(args.SelectedItem()));
                return;
            }
        }
    }

    void ExplorerPage::recentsButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        auto items = recentsMenuFlyout().Items();
        hstring currentPath = testBox().Text();
        for (auto item : items)
        {
            auto flyout = item.try_as<MenuFlyoutItem>();
            if (flyout && flyout.Text() == currentPath)
            {
                return;
            }
        }

        MenuFlyoutItem item{};
        item.Text(currentPath);
        recentsMenuFlyout().Items().Append(item);
    }

    void ExplorerPage::cutAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        MainWindow::Current().notifyUser(L"Cutting files to clipboard is not supported yet.", InfoBarSeverity::Error);
    }

    IAsyncAction ExplorerPage::copyAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        DataPackage dataPackage{};
        IVector<IStorageItem> items{};

        dataPackage.SetS;
        Clipboard::SetContent()
    }

    void ExplorerPage::renameAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {

    }

    void ExplorerPage::deleteAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {

    }


    void ExplorerPage::loadPath(hstring const& path)
    {
        testListView().Items().Clear();
        testProgressRing().Visibility(Visibility::Visible);

        WIN32_FIND_DATA data{};
        HANDLE handle = FindFirstFile((path + L"*").c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                int64_t size = static_cast<int64_t>(data.nFileSizeHigh) << 32;
                size += data.nFileSizeLow;
                testListView().Items().Append(FileEntryView(to_hstring(data.cFileName), to_hstring(data.cAlternateFileName), size, data.dwFileAttributes));
            } while (FindNextFile(handle, &data));
            FindClose(handle);
        }
        else
        {
            MainWindow::Current().notifyError(GetLastError());
        }

        testProgressRing().Visibility(Visibility::Collapsed);
        testBox().Text(path);
    }

    void ExplorerPage::completePath(hstring const& path, IVector<IInspectable> const& suggestions)
    {
        // complete with FindFirstFile and FindNextFile
        if (path.size() > 0)
        {
            WIN32_FIND_DATA data{};
            HANDLE findHandle = FindFirstFile((path + L"*").c_str(), &data);
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
            GetLogicalDriveStrings(512, drives);
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

    FileEntryView ExplorerPage::getSelectedItem()
    {
        TabViewItem selectedTab = tabView().SelectedItem().try_as<TabViewItem>();
        if (selectedTab)
        {
            ListView listView = selectedTab.FindName(L"testListView").try_as<ListView>();
            if (listView)
            {
                FileEntryView selectedItem = listView.SelectedItem().as<FileEntryView>();
                return selectedItem;
            }
        }
        
        return nullptr;
    }
}