#include "pch.h"
#include "ExplorerPage.xaml.h"
#if __has_include("ExplorerPage.g.cpp")
#include "ExplorerPage.g.cpp"
#include "DriveSearchModel.h"
#include "FileTabView.xaml.h"
#endif

using namespace winrt;
using namespace std;

using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    ExplorerPage::ExplorerPage()
    {
        // subscribing to the closed event because Page_Unloaded doesn't get called when the window is closed
        windowClosedToken = MainWindow::Current().Closed({ this, &ExplorerPage::mainWindow_Closed });
        InitializeComponent();
    }

    ExplorerPage::~ExplorerPage()
    {
        if (windowClosedToken)
        {
            // might fail if the window has completly been destroyed before an instance of this object
            MainWindow::Current().Closed(windowClosedToken);
        }
    }

    void ExplorerPage::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        // get last path
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");
        if (!settings)
        {
            settings = ApplicationData::Current().LocalSettings().CreateContainer(L"Explorer", ApplicationDataCreateDisposition::Always);
        }

        IReference<bool> regexChecked = settings.Values().TryLookup(L"UseSearchRegex").try_as<IReference<bool>>();
        if (regexChecked)
        {
            useRegexButton().IsChecked(regexChecked);
        }
        else
        {
            useRegexButton().IsChecked(IReference<bool>{ false });
        }

        ApplicationDataContainer recents = settings.Containers().TryLookup(L"ExplorerRecents");
        if (recents)
        {
            auto containers = recents.Values();
            for (auto const& c : containers)
            {
                hstring recent = unbox_value_or<hstring>(c.Value(), L"");
                if (recent != L"")
                {
                    MenuFlyoutItem item{};
                    item.Text(recent);
                    recentsMenuFlyout().Items().Append(item);
                }
            }
        }

        ApplicationDataContainer tabs = settings.Containers().TryLookup(L"ExplorerTabs");
        if (tabs)
        {
            auto containers = tabs.Values();
            for (auto const& c : containers)
            {
                ApplicationDataCompositeValue composite = c.Value().try_as<ApplicationDataCompositeValue>();
                hstring path = composite.Lookup(L"TabPath").as<hstring>();
                IInspectable tabName = composite.Lookup(L"TabName"); // not unboxing to save compute time/memory
                if (path != L"")
                {
                    TabViewItem tabViewItem{};
                    tabViewItem.Header(box_value(tabName));
                    tabViewItem.Content(Winnerino::FileTabView{ path });
                    tabView().TabItems().Append(tabViewItem);
                }
            }
        }
        // remove the container
        settings.DeleteContainer(L"ExplorerTabs");

        // list available drives
        WCHAR drives[512]{};
        WCHAR* pointer = drives;
        GetLogicalDriveStrings(512, drives);
        while (1)
        {
            if (*pointer == NULL)
            {
                break;
            }
            driveListView().Items().Append(Winnerino::DriveSearchModel{ hstring{ pointer } });
            while (*pointer++);
        }
    }

    void ExplorerPage::Page_Unloaded(IInspectable const& sender, RoutedEventArgs const& e)
    {
        savePage();
    }

    void ExplorerPage::recentsButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        TabViewItem selectedTab = tabView().SelectedItem().try_as<TabViewItem>();
        if (selectedTab)
        {
            UserControl test = selectedTab.Content().try_as<UserControl>();
            AutoSuggestBox box = test.FindName(L"pathInputBox").try_as<AutoSuggestBox>();
            if (box)
            {
                hstring currentPath = box.Text();

                if (currentPath == L"")
                {
                    return;
                }
                auto items = recentsMenuFlyout().Items();
                for (auto const& item : items)
                {
                    MenuFlyoutItem flyout = item.try_as<MenuFlyoutItem>();
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
                MenuFlyoutItem item{};
                item.Text(currentPath);
                recentsMenuFlyout().Items().Append(item);
            }
        }
    }

    void ExplorerPage::cutAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        MainWindow::Current().notifyUser(L"Cutting files to clipboard is not supported yet.", InfoBarSeverity::Error);
    }

    void ExplorerPage::copyAppBarButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        DataPackage dataPackage{};
        IVector<IStorageItem> items{};

        dataPackage.SetStorageItems(items);
        dataPackage.RequestedOperation(DataPackageOperation::Copy);
        Clipboard::SetContent(dataPackage);
        MainWindow::Current().notifyUser(L"File(s) copied to clipboard.", InfoBarSeverity::Success);
    }

    void ExplorerPage::renameAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
    }

    void ExplorerPage::deleteAppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        throw winrt::hresult_not_implemented();
    }

    void ExplorerPage::openFileTabButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        TabViewItem tab{};
        tab.Header(box_value(L"Empty"));
        tab.Content(Winnerino::FileTabView{});
        tabView().TabItems().Append(tab);
    }

    void ExplorerPage::tabView_TabCloseRequested(TabView const& sender, TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t indexOf;
        if (sender.TabItems().IndexOf(args.Item(), indexOf))
        {
            sender.TabItems().RemoveAt(indexOf); // why do we need to do this ? Can't the TabView do that itself ?
        }
    }

    void ExplorerPage::searchBox_GotFocus(IInspectable const&, RoutedEventArgs const&)
    {
        // use animation when done
        searchBox().HorizontalAlignment(HorizontalAlignment::Stretch);
        searchBox().Width(NAN);
    }

    void ExplorerPage::searchBox_LostFocus(IInspectable const&, RoutedEventArgs const&)
    {
        searchBox().HorizontalAlignment(HorizontalAlignment::Right);
        searchBox().Width(80);
    }


    IVector<FileEntryView> ExplorerPage::getSelectedItems()
    {
        IVector<FileEntryView> vect{ single_threaded_vector<FileEntryView>() };
        TabViewItem selectedTab = tabView().SelectedItem().try_as<TabViewItem>();
        if (selectedTab)
        {
            ListView listView = selectedTab.FindName(L"testListView").try_as<ListView>();
            if (listView)
            {
                auto const& selectedItems = listView.SelectedItems();
                for (IInspectable const& selectedItem : selectedItems)
                {
                    FileEntryView file = selectedItem.try_as<FileEntryView>();
                    if (file)
                    {
                        vect.Append(file);
                    }
                }
            }
        }
        
        return vect;
    }

    void ExplorerPage::savePage()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");

        settings.Values().Insert(L"UseSearchRegex", useRegexButton().IsChecked());

        HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
        ApplicationDataContainer tabContainer = settings.Containers().TryLookup(L"ExplorerTabs");
        if (!tabContainer)
        {
            tabContainer = settings.CreateContainer(L"ExplorerTabs", ApplicationDataCreateDisposition::Always);
        }

        IVector<IInspectable> tabItems = tabView().TabItems();
        for (IInspectable const& item : tabItems)
        {
            TabViewItem tab = item.try_as<TabViewItem>();
            if (tab)
            {
                UserControl control = tab.Content().try_as<Winnerino::FileTabView>();
                if (control)
                {
                    hstring tabName = tab.Header().as<hstring>();
                    hstring path = control.FindName(L"pathInputBox").as<AutoSuggestBox>().Text();

                    ApplicationDataCompositeValue composite{};
                    composite.Insert(L"TabName", box_value(tabName));
                    composite.Insert(L"TabPath", box_value(path));

                    hstring key = CryptographicBuffer::EncodeToHexString(
                        provider.HashData(CryptographicBuffer::ConvertStringToBinary(tabName + path, BinaryStringEncoding::Utf8)));

                    tabContainer.Values().Insert(key, composite);
                }
            }
        }
    }

    void ExplorerPage::mainWindow_Closed(IInspectable const&, WindowEventArgs const& args)
    {
        savePage();
    }
}