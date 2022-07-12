#include "pch.h"
#include "FilePropertiesWindow.xaml.h"
#if __has_include("FilePropertiesWindow.g.cpp")
#include "FilePropertiesWindow.g.cpp"
#endif

#include "FileSearcher.h"
#include "FileInfo.h"
#include "FileSearcher2.h"
#include "Helpers.h"

#define INVALIDATE_VIEW 0
#define RESET_TITLE_BAR 0

using namespace ::Winnerino::Storage;

using namespace winrt;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Composition;
using namespace winrt::Microsoft::UI::Composition::SystemBackdrops;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FilePropertiesWindow::FilePropertiesWindow()
    {
        InitializeComponent();
        InitWindow();

        mainWindowClosedEventToken = MainWindow::Current().Closed({ get_weak(), &FilePropertiesWindow::MainWindow_Closed});
        mainWindowThemeChangedToken = MainWindow::Current().ThemeChanged([&](auto const&, auto const& args)
        {
            RootGrid().RequestedTheme(args);
        });
    }

    FilePropertiesWindow::~FilePropertiesWindow()
    {
        MainWindow::Current().Closed(mainWindowClosedEventToken);
        MainWindow::Current().ThemeChanged(mainWindowThemeChangedToken);
        appWindow.Changed(appWindowChangedEventToken);
        appWindow.Closing(appWindowClosingToken);
        appWindow.Destroy();
    }

    void FilePropertiesWindow::Control_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        RootGrid().RequestedTheme((ElementTheme)unbox_value_or<int32_t>(ApplicationData::Current().LocalSettings().Values().TryLookup(L"AppTheme"), static_cast<int32_t>(ElementTheme::Default)));
    }

    void FilePropertiesWindow::RootGrid_SizeChanged(IInspectable const&, SizeChangedEventArgs const&)
    {
    }


    void FilePropertiesWindow::InitWindow()
    {
        auto nativeWindow{ this->try_as<::IWindowNative>() };
        winrt::check_bool(nativeWindow);
        HWND handle{ nullptr };
        nativeWindow->get_WindowHandle(&handle);

        WindowId windowID = GetWindowIdFromWindow(handle);
        appWindow = AppWindow::GetFromWindowId(windowID);
        // I18N: Window name -> File properties
        appWindow.Title(L"File properties");
        appWindow.MoveAndResize(RectInt32(100, 100, 350, 500));
        appWindowClosingToken = appWindow.Closing({ this, &FilePropertiesWindow::AppWindow_Closing });

        if (AppWindowTitleBar::IsCustomizationSupported())
        {
            appWindow.TitleBar().ExtendsContentIntoTitleBar(true);

            appWindow.TitleBar().ButtonBackgroundColor(Colors::Transparent());
            appWindow.TitleBar().ButtonForegroundColor(Colors::White());
            appWindow.TitleBar().ButtonInactiveBackgroundColor(Colors::Transparent());
            appWindow.TitleBar().ButtonInactiveForegroundColor(Colors::Gray());

            appWindow.TitleBar().ButtonHoverBackgroundColor(
                Application::Current().Resources().TryLookup(box_value(L"AppTitleBarHoverColor")).as<Windows::UI::Color>());

            appWindow.TitleBar().ButtonHoverForegroundColor(Colors::White());
            appWindow.TitleBar().ButtonPressedBackgroundColor(Colors::Transparent());
            appWindow.TitleBar().ButtonPressedForegroundColor(Colors::White());
        }
        else // hide title bar
        {
            TitleBarGrid().Visibility(Visibility::Collapsed);
            UnloadObject(TitleBarGrid());
            RootGrid().RowDefinitions().RemoveAt(0);
        }

        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"FilePropertiesWindow");
        if (!settings)
        {
            settings = ApplicationData::Current().LocalSettings().CreateContainer(L"FilePropertiesWindow", ApplicationDataCreateDisposition::Always);
        }

        if (unbox_value_or<bool>(settings.Values().TryLookup(L"UsesAcrylic"), true))
        {
            SetBackground();
        }
        
        auto&& presenter = appWindow.Presenter().as<OverlappedPresenter>();
        presenter.IsMaximizable(false);
        presenter.IsMinimizable(false);

        bool shown = unbox_value_or<bool>(settings.Values().TryLookup(L"ShowInSwitchers"), false);
        appWindow.IsShownInSwitchers(shown);
        presenter.IsAlwaysOnTop(!shown);
    }

    void FilePropertiesWindow::SetBackground()
    {
        if (DesktopAcrylicController::IsSupported())
        {
            auto&& supportsBackdrop = try_as<ICompositionSupportsSystemBackdrop>();
            if (supportsBackdrop)
            {
                if (!DispatcherQueue::GetForCurrentThread() && !dispatcherQueueController)
                {
                    DispatcherQueueOptions options
                    {
                        sizeof(DispatcherQueueOptions),
                        DQTYPE_THREAD_CURRENT,
                        DQTAT_COM_NONE
                    };

                    ABI::Windows::System::IDispatcherQueueController* ptr{ nullptr };
                    check_hresult(CreateDispatcherQueueController(options, &ptr));
                    dispatcherQueueController = { ptr, take_ownership_from_abi };
                }

                systemBackdropConfiguration = SystemBackdropConfiguration();
                systemBackdropConfiguration.IsInputActive(true);
                systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());

                activatedRevoker = Activated(auto_revoke, [this](IInspectable const&, WindowActivatedEventArgs const& args)
                {
                    systemBackdropConfiguration.IsInputActive(WindowActivationState::Deactivated != args.WindowActivationState());
                });

                themeChangedRevoker = RootGrid().ActualThemeChanged(auto_revoke, [this](FrameworkElement const&, IInspectable const&)
                {
                    systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());
                });


                backdropController = DesktopAcrylicController();
                //backdropController.TintColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                backdropController.FallbackColor(Application::Current().Resources().TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                backdropController.TintOpacity(0.3f);
                backdropController.LuminosityOpacity(0.001f);
                backdropController.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                backdropController.AddSystemBackdropTarget(supportsBackdrop);
            }
        }
    }

    void FilePropertiesWindow::MainWindow_Closed(IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&)
    {
        Close();
    }

    void FilePropertiesWindow::AppWindow_Closing(AppWindow const&, AppWindowClosingEventArgs const&)
    {
        if (backdropController)
        {
            backdropController.Close();
        }
        if (dispatcherQueueController)
        {
            dispatcherQueueController.ShutdownQueueAsync();
        }
    }
}