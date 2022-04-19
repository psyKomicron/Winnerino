#include "pch.h"
#include "TwitchPage2.xaml.h"
#if __has_include("TwitchPage2.g.cpp")
#include "TwitchPage2.g.cpp"
#endif

using namespace std;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Security::Cryptography;
using namespace winrt::Windows::Security::Cryptography::Core;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    TwitchPage2::TwitchPage2()
    {
        InitializeComponent();
        mainWindowClosedToken = MainWindow::Current().Closed({ this, &TwitchPage2::MainWindow_Closed });
    }

    TwitchPage2::~TwitchPage2()
    {
        MainWindow::Current().Closed(mainWindowClosedToken);
    }

    void TwitchPage2::Page_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        // load settings
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TwitchSettings");
        if (settings)
        {
            ApplicationDataContainer tabs = settings.Containers().TryLookup(L"WebTabs");
            if (tabs)
            {
                IPropertySet values = tabs.Values();
                for (auto const& value : values)
                {
                    ApplicationDataCompositeValue composite = value.Value().try_as<ApplicationDataCompositeValue>();
                    hstring path = unbox_value_or<hstring>(composite.Lookup(L"TabPath"), L"");
                    if (path != L"")
                    {
                        TabViewItem tabViewItem{};
                        tabViewItem.Header(box_value(L"Empty"));
                        tabViewItem.Content(WebTab{ path, tabViewItem });
                        webTabView().TabItems().Append(tabViewItem);
                    }
                }
                settings.DeleteContainer(L"WebTabs");
            }
        }
    }

    void TwitchPage2::webTabView_AddTabButtonClick(TabView const& sender, IInspectable const&)
    {
        TabViewItem tabViewItem{};
        tabViewItem.Header(box_value(L"Empty"));
        tabViewItem.Content(WebTab{ L"", tabViewItem });
        sender.TabItems().Append(tabViewItem);
    }

    void TwitchPage2::chatTabView_AddTabButtonClick(TabView const& sender, IInspectable const&)
    {
        TabViewItem tabViewItem{};
        tabViewItem.Header(box_value(L"Empty"));
        ChatView content{};
        tabViewItem.Content(content);
        sender.TabItems().Append(tabViewItem);
    }

    void TwitchPage2::TabView_TabCloseRequested(TabView const& sender, TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t index = 0;
        if (sender.TabItems().IndexOf(args.Item(), index))
        {
            sender.TabItems().RemoveAt(index);
        }
    }

    void TwitchPage2::Page_Unloaded(IInspectable const&, RoutedEventArgs const&)
    {
        SavePage();
    }

    void TwitchPage2::AppBarButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TwitchSettings");
        if (settings && settings.Containers().HasKey(L"WebTabs"))
        {
            settings.DeleteContainer(L"WebTabs");
            MainWindow::Current().NotifyUser(L"Cleared settings", InfoBarSeverity::Success);
        }
    }

    void TwitchPage2::AppBarButton_Click_1(IInspectable const&, RoutedEventArgs const&)
    {
        SavePage();
    }

    void TwitchPage2::SavePage()
    {
        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"TwitchSettings");
        if (!settings)
        {
            settings = ApplicationData::Current().LocalSettings().CreateContainer(L"TwitchSettings", ApplicationDataCreateDisposition::Always);
        }

        HashAlgorithmProvider hasher = HashAlgorithmProvider::OpenAlgorithm(HashAlgorithmNames::Sha256());
        ApplicationDataContainer webTabs = settings.CreateContainer(L"WebTabs", ApplicationDataCreateDisposition::Always);
        IVector<IInspectable> tabItems = webTabView().TabItems();
        for (IInspectable const& tab : tabItems)
        {
            TabViewItem tabViewItem = tab.try_as<TabViewItem>();
            if (tab)
            {
                UserControl control = tabViewItem.Content().try_as<WebTab>();
                if (control)
                {
                    hstring tabName = tabViewItem.Header().as<hstring>();
                    hstring path = control.FindName(L"searchSuggestBox").as<AutoSuggestBox>().Text();

                    if (!path.empty())
                    {
                        ApplicationDataCompositeValue composite{};
                        composite.Insert(L"TabPath", box_value(path));

                        hstring key = CryptographicBuffer::EncodeToHexString(
                            hasher.HashData(CryptographicBuffer::ConvertStringToBinary(tabName + path, BinaryStringEncoding::Utf8)));

                        webTabs.Values().Insert(key, composite);
                    }
                }
            }
        }
    }

    void TwitchPage2::MainWindow_Closed(IInspectable const&, WindowEventArgs const& args)
    {
        SavePage();
    }
}