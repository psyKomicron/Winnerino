#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI;
using namespace Microsoft::UI::Windowing;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Interop;
using namespace Windows::Graphics;
using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    Winnerino::MainWindow MainWindow::singleton{ nullptr };

    MainWindow::MainWindow()
    {
        InitializeComponent();
#if USING_TIMER
        dispatcherQueueTimer = DispatcherQueue().CreateTimer();
        dispatcherQueueTimer.Interval(TimeSpan{ 2s });
        dispatcherQueueTimer.IsRepeating(true);
        dispatcherQueueTimer.Tick({ this, &MainWindow::dispatcherQueueTimer_Tick });
#endif // USING_TIMER

        singleton = *this;
        InitWindow();
    }

    Windows::Foundation::Size MainWindow::Size() const
    {
        Windows::Foundation::Size size{};
        size.Width = static_cast<float>(appWindow.Size().Width);
        size.Height = static_cast<float>(appWindow.Size().Height);
        return size;
    }

    void MainWindow::NotifyUser(hstring const& message, InfoBarSeverity const& severity)
    {
        if (!busy)
        {
            if (DispatcherQueue().HasThreadAccess())
            {
                UpdateInforBar(message, severity);
            }
            else
            {
                MessageData* messageData = new MessageData{ message, severity };
                DispatcherQueue().TryEnqueue([this, messageData]()
                {
                    UpdateInforBar(messageData->GetDataMessage(), messageData->GetSeverity());
                    delete messageData;
                });
            }
            busy = true;
        }
        else
        {
            messagesStack.push(MessageData{ message, severity });
#if USING_TIMER
            if (!dispatcherQueueTimer.IsRunning())
            {
                dispatcherQueueTimer.Start();
            }
#endif // USING_TIMER

        }
    }

    void MainWindow::NotifyError(DWORD const& code)
    {
        hstring message = to_hstring(std::system_category().message(code));
#ifdef _DEBUG
        OutputDebugString((message + L"\n").c_str());
#endif // _DEBUG

        NotifyUser(message, InfoBarSeverity::Error);
    }

    void MainWindow::NotifyError(DWORD const& code, hstring const& additionalMessage)
    {
        hstring message = additionalMessage + L": " + to_hstring(std::system_category().message(code));
#ifdef _DEBUG
        OutputDebugString((message + L"\n").c_str());
#endif // _DEBUG

        NotifyUser(message, InfoBarSeverity::Error);
    }

    void MainWindow::ChangeTheme(ElementTheme const& theme)
    {
        contentGrid().RequestedTheme(theme);
        ApplicationData::Current().LocalSettings().Values().Insert(L"AppTheme", box_value(static_cast<int32_t>(theme)));
    }

    bool MainWindow::NavigateTo(TypeName const& typeName)
    {
        return ContentFrame().IsLoaded() && ContentFrame().Navigate(typeName);
    }

    bool MainWindow::NavigateTo(TypeName const& typeName, IInspectable const& parameter)
    {
        return ContentFrame().IsLoaded() && ContentFrame().Navigate(typeName, parameter);
    }

    void MainWindow::GoBack()
    {
        if (ContentFrame().IsLoaded() && ContentFrame().CanGoBack())
        {
            ContentFrame().GoBack();
        }
    }


    void MainWindow::View_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        IPropertySet settings = ApplicationData::Current().LocalSettings().Values();

        IInspectable inspectable = settings.TryLookup(L"LoadLastPage");
        if (unbox_value_or<bool>(inspectable, true))
        {
            //ApplicationDataContainer pageContainer = .try_as<ApplicationDataContainer>();
            hstring savedTag = unbox_value_or<hstring>(settings.TryLookup(L"LastPage"), L"Home");
            if (!LoadPage(savedTag))
            {
                NotifyUser(L"Failed to load previous session last page.", InfoBarSeverity::Warning);
            }
        }
        else
        {
            ContentFrame().Navigate(xaml_typename<Winnerino::MainPage>());
        }
    }

    void MainWindow::navigationView_ItemInvoked(NavigationView const&, NavigationViewItemInvokedEventArgs const& args)
    {
        if (args.InvokedItemContainer())
        {
            hstring tag = args.InvokedItemContainer().Tag().as<hstring>();
            LoadPage(tag);
        }
    }

    void MainWindow::appWindow_Closing(AppWindow const&, AppWindowClosingEventArgs const&)
    {
        SaveWindowState();
    }

    void MainWindow::appWindow_Changed(AppWindow const&, AppWindowChangedEventArgs const&)
    {
        if (appWindow.Presenter().Kind() == AppWindowPresenterKind::Overlapped)
        {
            OverlappedPresenter presenter = appWindow.Presenter().as<OverlappedPresenter>();
            isWindowFullscreen = presenter.State() == OverlappedPresenterState::Maximized;
        }
    }

    void MainWindow::InfoBar_Closed(InfoBar const&, InfoBarClosedEventArgs const&)
    {
#if USING_TIMER
        NotifInfoBar().Severity(InfoBarSeverity::Informational);
        NotifInfoBar().Message(L"");
        NotifInfoBar().Title(L"");
#else
        if (!messagesStack.empty())
        {
            MessageData message = messagesStack.front();
            messagesStack.pop();

            UpdateInforBar(message.GetDataMessage(), message.GetSeverity());
        }
        else
        {
            busy = false;
            NotifInfoBar().Severity(InfoBarSeverity::Informational);
            NotifInfoBar().Message(L"");
            NotifInfoBar().Title(L"");
    }
#endif // USING_TIMER
    }

#if USING_TIMER
    void MainWindow::dispatcherQueueTimer_Tick(Microsoft::UI::Dispatching::DispatcherQueueTimer const&, Windows::Foundation::IInspectable const&)
    {
        if (!messagesStack.empty())
        {
            MessageData message = messagesStack.front();
            messagesStack.pop();

            UpdateInforBar(message.GetDataMessage(), message.GetSeverity());
        }
        else
        {
            busy = false;
            dispatcherQueueTimer.Stop();
        }
    }
#endif // USING_TIMER

    void MainWindow::UpdateInforBar(hstring const& message, InfoBarSeverity const& severity)
    {
        NotifInfoBar().Severity(severity);
        NotifInfoBar().Message(message);

        switch (severity)
        {
            case InfoBarSeverity::Error:
                NotifInfoBar().Title(L"Error:");
                break;
            case InfoBarSeverity::Warning:
                NotifInfoBar().Title(L"Warning:");
                break;
            case InfoBarSeverity::Success:
                NotifInfoBar().Title(L"Success:");
                break;
            case InfoBarSeverity::Informational:
                NotifInfoBar().Title(L"Information:");
                break;
        }
        NotifInfoBar().IsOpen(true);
    }

    void MainWindow::InitWindow()
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

        contentGrid().RequestedTheme((ElementTheme)unbox_value_or<int32_t>(settings.Values().TryLookup(L"AppTheme"), static_cast<int32_t>(ElementTheme::Default)));
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
            appWindow.SetIcon(L"Images/multitool.ico");
            appWindow.Title(L"Multitool");

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
                //appWindow.TitleBar().PreferredHeightOption(TitleBarHeightOption::Tall);

                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);

                //appWindow.TitleBar().ButtonBackgroundColor(Colors::Black());
                appWindow.TitleBar().ButtonBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                appWindow.TitleBar().ButtonForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonInactiveForegroundColor(Colors::Gray());

                appWindow.TitleBar().ButtonHoverBackgroundColor(
                    Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());

                appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
                appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());
                appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
            }
        }

#ifdef _DEBUG
        VersionTextBlock().Text(L"PREVIEW");
#endif // 

    }

    void MainWindow::SaveWindowState()
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

    bool MainWindow::LoadPage(hstring page)
    {
        bool recognized = false; // remove when application is built enough
        if (page.empty() || page == L"Home")
        {
            ContentFrame().Navigate(xaml_typename<Winnerino::ExplorerPage>());
            recognized = true;
        }
        else if (page == L"Settings")
        {
            ContentFrame().Navigate(xaml_typename<Winnerino::SettingsPage>());
            recognized = true;
        }
        else if (page == L"NewContent")
        {
            ContentFrame().Navigate(xaml_typename<Winnerino::MainPage>());
            recognized = true;
        }

        if (recognized)
        {
            lastPage = page;
            Title(page);
        }
        return recognized;
    }

    void MainWindow::SetDragRectangles()
    {
        /*HWND windowHandle = GetWindowFromWindowId(appWindow.Id());
        if (windowHandle != INVALID_HANDLE_VALUE)
        {
            uint8_t appTitleBar = 32;
            uint8_t leftAdditionalPadding = 0;
            UINT windowDpi = GetDpiForWindow(windowHandle);

            double scale = (windowDpi * static_cast<double>(100) + (static_cast<double>(96) / 2)) / 96;
            scale /= static_cast<double>(100);

            double leftPadding = appWindow.TitleBar().LeftInset() / scale;
            double rightPadding = appWindow.TitleBar().RightInset() / scale;

            std::vector<RectInt32> rectVector{};

            RectInt32 dragRectangleLeft{};
            dragRectangleLeft.X = leftPadding;
            dragRectangleLeft.Y = 0;
            dragRectangleLeft.Height = appTitleBar * scale;
            dragRectangleLeft.Width = (leftAdditionalPadding + leftPadding) * scale;
            rectVector.push_back(dragRectangleLeft);

            RectInt32 dragRectangle{};
            dragRectangleLeft.X = (rightPadding + ) + scale;
            dragRectangleLeft.Y = 0;
            dragRectangleLeft.Height = appTitleBar * scale;
            dragRectangleLeft.Width = (leftAdditionalPadding + leftPadding) * scale;
            rectVector.push_back(dragRectangleLeft);

            appWindow.TitleBar().SetDragRectangles(vect);
        }*/
    }
}
