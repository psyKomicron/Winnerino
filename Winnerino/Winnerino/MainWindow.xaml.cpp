#include "pch.h"
#include "winrt/Windows.Storage.h"
#include "winrt/Microsoft.UI.Dispatching.h"

#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI;
using namespace Microsoft::UI::Windowing;

using namespace Windows::Graphics;
using namespace Windows::Storage;
using namespace Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    Winnerino::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow()
    {
        InitializeComponent();
        singleton = *this;
        initWindow();
    }

    void MainWindow::notifyUser(IInspectable const& message, InfoBarSeverity const& severity)
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

    void MainWindow::updateInforBar(IInspectable const& message,  InfoBarSeverity const& severity)
    {
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
        if (!isWindowFullscreen && appWindow != nullptr)
        {
            ApplicationDataContainer settings = ApplicationData::Current().LocalSettings();

            ApplicationDataCompositeValue setting = nullptr;
            if (settings.Values().HasKey(L"windowSize"))
            {
                setting = settings.Values().Lookup(L"windowSize").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"width", box_value(appWindow.Size().Width));
            setting.Insert(L"height", box_value(appWindow.Size().Height));
            settings.Values().Insert(L"windowSize", setting);

            setting = nullptr;
            if (settings.Values().HasKey(L"windowPosition"))
            {
                setting = settings.Values().Lookup(L"windowPosition").as<ApplicationDataCompositeValue>();
            }
            else
            {
                setting = ApplicationDataCompositeValue{};
            }
            setting.Insert(L"positionX", box_value(appWindow.Position().X));
            setting.Insert(L"positionY", box_value(appWindow.Position().Y));
            settings.Values().Insert(L"windowPosition", setting);
        }
    }

#pragma region EventHandlers
    void MainWindow::navigationView_Loaded(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
    }

    void MainWindow::navigationView_ItemInvoked(NavigationView const&, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args)
    {
        if (args.InvokedItemContainer())
        {
            hstring tag = args.InvokedItemContainer().Tag().as<hstring>();

            if (tag == L"Settings")
            {
                // navigate to settings page.
                contentFrame().Navigate(winrt::xaml_typename<Winnerino::SettingsPage>());
            }
            else if (tag == L"Home")
            {
            }
            else if (tag == L"Twitch")
            {
                contentFrame().Navigate(winrt::xaml_typename<Winnerino::TwitchPage2>());
            }
            else if (tag == L"Chat")
            {
            }
            else if (tag == L"Widgets")
            {
                contentFrame().Navigate(winrt::xaml_typename<Winnerino::WidgetsPage>());
            }
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
        bool presenterChanged = args.DidPresenterChange();
        bool positionChanged = args.DidPositionChange();
        bool sizeChanged = args.DidSizeChange();
        bool visibilityChanged = args.DidVisibilityChange();

        if (appWindow.Presenter().Kind() == AppWindowPresenterKind::Overlapped)
        {
            OverlappedPresenter presenter = appWindow.Presenter().as<OverlappedPresenter>();
            isWindowFullscreen = presenter.State() == OverlappedPresenterState::Maximized;
        }
    }

    void MainWindow::changePresenterAppBarButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        OutputDebugString(L"Change presenter.\n");
    }
#pragma endregion

}