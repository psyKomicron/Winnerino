#include "pch.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Microsoft.UI.Dispatching.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;

using namespace Microsoft::UI;
using namespace Microsoft::UI::Windowing;
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
#pragma endregion

#pragma region private
    void MainWindow::updateInforBar(hstring const& message, InfoBarSeverity const& severity)
    {
        infoBar().Severity(severity);
#if false
        infoBarContent().Text(message);
#else
        infoBar().Message(message);
#endif // _DEBUG

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
        int width = 800;
        int height = 600;
        int y = 50;
        int x = 50;

        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();
        IInspectable windowSize = settings.Values().TryLookup(L"windowSize");
        if (windowSize != nullptr)
        {
            ApplicationDataCompositeValue composite = windowSize.as<ApplicationDataCompositeValue>();
            IInspectable widthBoxed = composite.Lookup(L"width");
            IInspectable heightBoxed = composite.Lookup(L"height");
            if (widthBoxed)
            {
                width = unbox_value<int>(widthBoxed);
            }
            if (heightBoxed)
            {
                height = unbox_value<int>(heightBoxed);
            }
        }
        IInspectable windowPosition = settings.Values().TryLookup(L"windowPosition");
        if (windowSize != nullptr)
        {
            ApplicationDataCompositeValue composite = windowPosition.as<ApplicationDataCompositeValue>();
            IInspectable posX = composite.TryLookup(L"positionX");
            IInspectable posY = composite.Lookup(L"positionY");
            if (posX)
            {
                x = unbox_value<int>(posX);
            }
            if (posY)
            {
                y = unbox_value<int>(posY);
            }
        }

        auto nativeWindow{ this->try_as<::IWindowNative>() };
        winrt::check_bool(nativeWindow);
        HWND handle{ nullptr };
        nativeWindow->get_WindowHandle(&handle);
        if (nativeWindow == nullptr)
        {
            OutputDebugString(L"Failed to get window handle.");
            return;
        }

        //SetWindowPos(handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);

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
        }
    }

    void MainWindow::saveWindowState()
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();
        if (!isWindowFullscreen && appWindow != nullptr)
        {

            ApplicationDataCompositeValue setting = nullptr;
            if (settings.HasKey(L"windowSize"))
            {
                setting = settings.Lookup(L"windowSize").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"width", box_value(appWindow.Size().Width));
            setting.Insert(L"height", box_value(appWindow.Size().Height));
            settings.Insert(L"windowSize", setting);

            setting = nullptr;
            if (settings.HasKey(L"windowPosition"))
            {
                setting = settings.Lookup(L"windowPosition").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"positionX", box_value(appWindow.Position().X));
            setting.Insert(L"positionY", box_value(appWindow.Position().Y));
            settings.Insert(L"windowPosition", setting);
        }

        settings.Insert(L"lastPage", box_value(lastPage));
    }

    bool MainWindow::loadPage(hstring page)
    {
        bool recognized = false; // remove when application is built enough
        if (page == L"Settings")
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::SettingsPage>());
            recognized = true;
        }
        else if (page == L"Home")
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::MainPage>());
            recognized = true;
        }
        else if (page == L"Twitch")
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::TwitchPage2>());
            recognized = true;
        }
        else if (page == L"Chat")
        {
            notifyUser(L"Uh oh, this one is not done yet... Try again after an update !", InfoBarSeverity::Error);
            recognized = true;
        }
        else if (page == L"Widgets")
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::WidgetsPage>());
            recognized = true;
        }
        else if (page == L"Power")
        {
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::PowerModePage>());
            recognized = true;
        }

        if (recognized)
        {
            lastPage = page;
        }
        return recognized;
    }
#pragma endregion

#pragma region EventHandlers
    void MainWindow::navigationView_Loaded(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();
#if defined _DEBUG
        hstring pageContainer = unbox_value<hstring>(settings.TryLookup(L"lastPage"));
        if (!loadPage(pageContainer))
        {
            notifyUser(L"Failed to load previous session last page.", InfoBarSeverity::Error);
            contentFrame().Navigate(winrt::xaml_typename<Winnerino::MainPage>());
        }
#else
        IInspectable inspectable = settings.TryLookup(L"loadLastPage");
        if (inspectable)
        {
            //ApplicationDataContainer container = inspectable.try_as<ApplicationDataContainer>();
            if (unbox_value_or<bool>(inspectable, false))
            {
                ApplicationDataContainer pageContainer = settings.TryLookup(L"lastPage").try_as<ApplicationDataContainer>();
                if (!loadPage(unbox_value_or<hstring>(pageContainer, L"Home")))
                {
                    notifyUser(L"Failed to load previous session last page.", InfoBarSeverity::Error);
                }
            }
            else
            {
                contentFrame().Navigate(winrt::xaml_typename<Winnerino::MainPage>());
            }
        }
#endif // defined _RELEASE

    }

    void MainWindow::navigationView_ItemInvoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& args)
    {
        if (args.InvokedItemContainer())
        {
            hstring tag = args.InvokedItemContainer().Tag().as<hstring>();
            loadPage(tag);
        }
    }

    void MainWindow::Window_Closed(IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const& args)
    {
        //settings.Close();
    }

    void MainWindow::Window_SizeChanged(IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowSizeChangedEventArgs const& args)
    {
        /*width = args.Size().Width;
        height = args.Size().Height;*/
    }

    void MainWindow::appWindow_Closing(AppWindow const&, AppWindowClosingEventArgs const& args)
    {
        saveWindowState();
    }

    void MainWindow::appWindow_Changed(AppWindow const&, AppWindowChangedEventArgs const& args)
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
}
