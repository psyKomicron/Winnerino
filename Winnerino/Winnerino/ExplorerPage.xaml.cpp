#include "pch.h"
#include "ExplorerPage.xaml.h"
#if __has_include("ExplorerPage.g.cpp")
#include "ExplorerPage.g.cpp"
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
        if (settings)
        {
            ApplicationDataContainer tabs = settings.Containers().TryLookup(L"ExplorerTabs");
            if (tabs)
            {
                auto containers = tabs.Values();
                for (auto const& c : containers)
                {
                    ApplicationDataCompositeValue composite = c.Value().try_as<ApplicationDataCompositeValue>();
                    hstring path = unbox_value_or<hstring>(composite.Lookup(L"TabPath"), L"");
                    hstring tabName = unbox_value_or<hstring>(composite.Lookup(L"TabName"), L"Empty");
                    
                    AddTab(tabName, path);
                }
            }
            // remove the container
            settings.DeleteContainer(L"ExplorerTabs");
        }
        else
        {
            ApplicationData::Current().LocalSettings().CreateContainer(L"Explorer", ApplicationDataCreateDisposition::Always);   
        }

        if (FilesTabView().TabItems().Size() == 0)
        {
            AddTab(L"Empty", L"");
        }
    }

    void ExplorerPage::Page_Unloaded(IInspectable const&, RoutedEventArgs const&)
    {
        SavePage();
    }

    void ExplorerPage::TabView_TabCloseRequested(TabView const& sender, TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t indexOf;
        if (sender.TabItems().IndexOf(args.Item(), indexOf))
        {
            sender.TabItems().RemoveAt(indexOf); // why do we need to do this ? Can't the TabView do that itself ?
        }
    }

    void ExplorerPage::TabView_AddTabButtonClick(TabView const&, IInspectable const&)
    {
        AddTab(L"Empty", L"");
    }

    void ExplorerPage::AddTab(hstring const& header, hstring const& path)
    {
        TabViewItem tab{};
        TabViewItemHeader tabViewHeader{ header };
        tab.Header(tabViewHeader);

        if (path.empty())
        {
            tab.Content(FileTabView{});
        }
        else
        {
            tab.Content(FileTabView{ path });
        }

        uint32_t tabIndex = FilesTabView().TabItems().Size();
        FilesTabView().TabItems().Append(tab);
        FilesTabView().SelectedIndex(tabIndex);
    }

    void ExplorerPage::mainWindow_Closed(IInspectable const&, WindowEventArgs const&)
    {
        SavePage();
    }

    void ExplorerPage::SavePage()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"Explorer");

        HashAlgorithmProvider provider = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
        ApplicationDataContainer tabContainer = settings.CreateContainer(L"ExplorerTabs", ApplicationDataCreateDisposition::Always);

        IVector<IInspectable> tabItems = FilesTabView().TabItems();
        for (IInspectable const& item : tabItems)
        {
            TabViewItem tab = item.try_as<TabViewItem>();
            if (tab)
            {
                UserControl control = tab.Content().try_as<Winnerino::FileTabView>();
                if (control)
                {
                    hstring tabName = tab.Header().as<TabViewItemHeader>().Text();
                    hstring path = control.FindName(L"PathInputBox").as<AutoSuggestBox>().Text();

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
}