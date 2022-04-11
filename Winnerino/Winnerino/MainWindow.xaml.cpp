#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "winrt/Microsoft.UI.Dispatching.h"


using namespace winrt;

using namespace Microsoft::UI;
using namespace Microsoft::UI::Windowing;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

using namespace Windows::Graphics;
using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::Foundation::Collections;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    Winnerino::MainWindow MainWindow::singleton{ nullptr };

#pragma region public
    MainWindow::MainWindow()
    {
        InitializeComponent();
        Title(L"Multitool");
        singleton = *this;
        initWindow();
    }

    void MainWindow::notifyUser(hstring const& message, InfoBarSeverity const& severity)
    {
        if (singleton.DispatcherQueue().HasThreadAccess())
        {
            updateInforBar(message, severity);
        }
        else
        {
            DispatcherQueue().TryEnqueue([this, message, severity]()
                {
                    updateInforBar(message, severity);
                });
        }
    }

    void MainWindow::notifyError(DWORD code)
    {
        notifyUser(to_hstring(std::system_category().message(code)), InfoBarSeverity::Error);
    }

#pragma region handlers
    void MainWindow::navigationView_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();

        IInspectable inspectable = settings.TryLookup(L"LoadLastPage");
        if (unbox_value_or<bool>(inspectable, false))
        {
            //ApplicationDataContainer pageContainer = .try_as<ApplicationDataContainer>();
            if (!loadPage(unbox_value_or<hstring>(settings.TryLookup(L"LastPage"), L"Home")))
            {
                notifyUser(L"Failed to load previous session last page.", InfoBarSeverity::Warning);
            }
        }
        else
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::MainPage>());
        }
    }

    void MainWindow::navigationView_ItemInvoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& args)
    {
        if (args.InvokedItemContainer())
        {
            hstring tag = args.InvokedItemContainer().Tag().as<hstring>();
            loadPage(tag);
        }
    }

    void MainWindow::appWindow_Closing(AppWindow const&, AppWindowClosingEventArgs const&)
    {
        saveWindowState();
    }

    void MainWindow::appWindow_Changed(AppWindow const&, AppWindowChangedEventArgs const&)
    {
        if (appWindow.Presenter().Kind() == AppWindowPresenterKind::Overlapped)
        {
            OverlappedPresenter presenter = appWindow.Presenter().as<OverlappedPresenter>();
            isWindowFullscreen = presenter.State() == OverlappedPresenterState::Maximized;
        }
    }

    void MainWindow::infoBar_Closed(InfoBar const&, InfoBarClosedEventArgs const&)
    {
        infoBar().Severity(InfoBarSeverity::Informational);
        infoBar().Message(L"");
        infoBar().Title(L"");
    }
#pragma endregion

#pragma endregion

#pragma region private
    void MainWindow::updateInforBar(hstring const& message, InfoBarSeverity const& severity)
    {
        infoBar().Severity(severity);
        infoBar().Message(message);

        switch (severity)
        {
            case InfoBarSeverity::Error:
                infoBar().Title(L"Error:");
                break;
            case InfoBarSeverity::Warning:
                infoBar().Title(L"Warning:");
                break;
            case InfoBarSeverity::Success:
                infoBar().Title(L"Success:");
                break;
            case InfoBarSeverity::Informational:
                infoBar().Title(L"Information:");
                break;
        }
        infoBar().IsOpen(true);
    }

    void MainWindow::initWindow()
    {
#pragma region settings
        int width = 800;
        int height = 600;
        int y = 50;
        int x = 50;

        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();
        IInspectable windowSize = settings.Values().TryLookup(L"WindowSize");
        if (windowSize != nullptr)
        {
            ApplicationDataCompositeValue composite = windowSize.as<ApplicationDataCompositeValue>();
            IInspectable widthBoxed = composite.Lookup(L"Width");
            IInspectable heightBoxed = composite.Lookup(L"Height");
            if (widthBoxed)
            {
                width = unbox_value<int>(widthBoxed);
            }
            if (heightBoxed)
            {
                height = unbox_value<int>(heightBoxed);
            }
        }
        IInspectable windowPosition = settings.Values().TryLookup(L"WindowPosition");
        if (windowSize != nullptr)
        {
            ApplicationDataCompositeValue composite = windowPosition.as<ApplicationDataCompositeValue>();
            IInspectable posX = composite.TryLookup(L"PositionX");
            IInspectable posY = composite.Lookup(L"PositionY");
            if (posX)
            {
                x = unbox_value<int>(posX);
            }
            if (posY)
            {
                y = unbox_value<int>(posY);
            }
        }
#pragma endregion

        auto nativeWindow{ this->try_as<::IWindowNative>() };
        winrt::check_bool(nativeWindow);
        HWND handle{ nullptr };
        nativeWindow->get_WindowHandle(&handle);
        if (nativeWindow == nullptr)
        {
            OutputDebugString(L"Failed to get window handle.");
            return;
        }

        WindowId windowID = GetWindowIdFromWindow(handle);
        appWindow = AppWindow::GetFromWindowId(windowID);
        if (appWindow != nullptr)
        {
            RectInt32 rect{};
            rect.Height = height;
            rect.Width = width;
            rect.X = x;
            rect.Y = y;
            appWindow.MoveAndResize(rect);
            appWindow.Closing({ get_weak(), &MainWindow::appWindow_Closing });
            appWindow.Changed({ get_weak(), &MainWindow::appWindow_Changed });

            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);

                //appWindow.TitleBar().ButtonBackgroundColor = Tool.GetAppRessource<Color>("DarkBlack");
                appWindow.TitleBar().ButtonBackgroundColor(Colors::Black());
                appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(Colors::Gray());

                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());

                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
            }
            else //hide title bar
            {
                uint32_t index = 0;
                if (contentGrid().Children().IndexOf(titleBarGrid(), index))
                {
                    contentGrid().Children().RemoveAt(index);
                }
                contentGrid().RowDefinitions().RemoveAt(0);
            }
        }
    }

    void MainWindow::saveWindowState()
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();
        if (!isWindowFullscreen && appWindow != nullptr)
        {
            ApplicationDataCompositeValue setting = nullptr;
            if (settings.HasKey(L"WindowSize"))
            {
                setting = settings.Lookup(L"WindowSize").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"Width", box_value(appWindow.Size().Width));
            setting.Insert(L"Height", box_value(appWindow.Size().Height));
            settings.Insert(L"WindowSize", setting);

            setting = nullptr;
            if (settings.HasKey(L"WindowPosition"))
            {
                setting = settings.Lookup(L"WindowPosition").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"PositionX", box_value(appWindow.Position().X));
            setting.Insert(L"PositionY", box_value(appWindow.Position().Y));
            settings.Insert(L"WindowPosition", setting);
        }

        settings.Insert(L"LastPage", box_value(lastPage));
    }

    bool MainWindow::loadPage(hstring page)
    {
        bool recognized = false; // remove when application is built enough
        if (page == L"Settings")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::SettingsPage>());
            recognized = true;
        }
        else if (page == L"Home")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::MainPage>());
            recognized = true;
        }
        else if (page == L"Twitch")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::TwitchPage2>());
            recognized = true;
        }
        else if (page == L"Chat")
        {
            notifyUser(L"Uh oh, this one is not done yet... Try again after an update !", InfoBarSeverity::Error);
        }
        else if (page == L"Widgets")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::WidgetsPage>());
            recognized = true;
        }
        else if (page == L"Power")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::PowerModePage>());
            recognized = true;
        }
        else if (page == L"Explorer")
        {
            contentFrame().Navigate(xaml_typename<Winnerino::ExplorerPage>());
            recognized = true;
        }

        if (recognized)
        {
            lastPage = page;
            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                loadedPageText().Text(page);
            }
            Title(page);
        }
        return recognized;
    }
#pragma endregion
}
