#include "pch.h"
#include "FileSearchWindow.xaml.h"
#if __has_include("FileSearchWindow.g.cpp")
#include "FileSearchWindow.g.cpp"
#endif

#include "FileSearcher.h"
#include "FileInfo.h"
#include "FileSearcher2.h"
#include "Helper.h"

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
    FileSearchWindow::FileSearchWindow()
    {
        InitializeComponent();
        InitWindow();
        mainWindowClosedEventToken = MainWindow::Current().Closed({ this, &FileSearchWindow::MainWindow_Closed });
    }

    FileSearchWindow::~FileSearchWindow()
    {
        MainWindow::Current().Closed(mainWindowClosedEventToken);
        appWindow.Changed(appWindowChangedEventToken);
        appWindow.Closing(appWindowClosingToken);
        appWindow.Destroy();
    }

    void FileSearchWindow::SetPosition(winrt::Windows::Graphics::PointInt32 const& position)
    {
        if (appWindow)
        {
            appWindow.Move(position);
        }
    }

    void FileSearchWindow::OnLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        if (AppWindowTitleBar::IsCustomizationSupported() && appWindow.TitleBar().ExtendsContentIntoTitleBar())
        {
            SetDragRectangles();
        }
    }

    void FileSearchWindow::RootGrid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e)
    {
        SetDragRectangles();
    }

    void FileSearchWindow::AutoSuggestBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args)
    {
        if (!args.QueryText().empty())
        {
            if (SearchResults().Items().Size() > 0)
            {
                SearchResults().Items().Clear();
            }
            SearchProgressRing().Visibility(Visibility::Visible);

            concurrency::create_task([this, path = args.QueryText().c_str(), root = RootTextBox().Text()]()
            {
#if TRUE
                FileSearcher2 searcher{ root };
                vector<FileInfo>* results = new vector<FileInfo>();
#else
                FileSearcher searcher{};
                if (!root.empty())
                {
                    searcher.Root(root);
                }
                vector<hstring> results{};
                searcher.MatchFound({ get_weak(), &FilePropertiesWindow::MatchFound });
#endif // _DEBUG


                searcher.Search(wregex{ path }, results);
                OutputDebugString(L"Finished searching\n");

                DispatcherQueue().TryEnqueue([this]()
                {
                    SearchResults().Items().Clear();
                });


#if TRUE
                DispatcherQueue().TryEnqueue([this, results]()
                {
                    for (size_t i = 0; i < results->size(); i++)
                    {
                        FileInfo file = results->at(i);
                        FileEntryView view = FileEntryView{ file.Name(), file.Path(), file.Size(), file.Attributes() };
                        view.ShowFilePath(true);
                        SearchResults().Items().Append(view);
                    }
                    delete results;
                });
#else
                for (auto&& result : results)
                {
                    DispatcherQueue().TryEnqueue([this, &file = result]()
                    {
                        TextBlock text{};
                        text.TextWrapping(TextWrapping::Wrap);
                        text.IsTextSelectionEnabled(true);
                        text.Text(filePath);
                        SearchResults().Items().Append(text);
                    });
                }
#endif // _DEBUG  

                DispatcherQueue().TryEnqueue([this]()
                {
                    SearchProgressRing().Visibility(Visibility::Collapsed);
                });
            });
        }
        else
        {
            //Close();
        }
    }


    void FileSearchWindow::InitWindow()
    {
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
        // I18N: Window name -> Search file system
        appWindow.Title(L"Search file system");
        appWindow.MoveAndResize(RectInt32(100, 100, 800, 500));

        appWindowClosingToken = appWindow.Closing({ this, &FileSearchWindow::AppWindow_Closing });

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

            appWindow.TitleBar().PreferredHeightOption(TitleBarHeightOption::Tall);
        }
        else // hide title bar
        {
            /*uint32_t index = 0;
            if (contentGrid().Children().IndexOf(titleBarGrid(), index))
            {
                contentGrid().Children().RemoveAt(index);
            }
            contentGrid().RowDefinitions().RemoveAt(0);*/
        }

        ApplicationDataContainer settings = ApplicationData::Current().LocalSettings().Containers().TryLookup(L"FileSearchWindow");
        if (!settings)
        {
            settings = ApplicationData::Current().LocalSettings().CreateContainer(L"FileSearchWindow", ApplicationDataCreateDisposition::Always);
        }

        if (unbox_value_or(ApplicationData::Current().LocalSettings().Values().TryLookup(L"WindowsCanUseTransparencyEffects"), true) && 
            unbox_value_or(settings.Values().TryLookup(L"UsesAcrylic"), true))
        {
            SetBackground((::Winnerino::DesktopTransparencyControllerType)unbox_value_or<uint8_t>(
                ApplicationData::Current().LocalSettings().Values().TryLookup(L"TransparencyEffectController"), 
                ::Winnerino::DesktopTransparencyControllerType::Acrylic)
            );
        }

        auto&& presenter = appWindow.Presenter().as<OverlappedPresenter>();
        presenter.IsMaximizable(false);
        presenter.IsMinimizable(false);

        presenter.IsAlwaysOnTop(unbox_value_or<bool>(settings.Values().TryLookup(L"KeepOnTop"), false));
    }

    void FileSearchWindow::SetDragRectangles()
    {
#if RESET_TITLE_BAR
        appWindow.TitleBar().ResetToDefault();
        appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
#endif // DEBUG

        HWND windowHandle = GetWindowFromWindowId(appWindow.Id());
        UINT windowDpi = GetDpiForWindow(windowHandle);

        double dpi = (windowDpi * static_cast<double>(100) + (static_cast<double>(96) / 2)) / 96;
        double scale = dpi / static_cast<double>(100);

        int32_t appTitleBarHeight = static_cast<int32_t>(appWindow.TitleBar().Height() * scale);

        double leftPadding = appWindow.TitleBar().LeftInset() / scale;
        double rightPadding = appWindow.TitleBar().RightInset() / scale;

        RightPaddingColumn().Width(GridLength{ rightPadding });
        LeftPaddingColumn().Width(GridLength{ leftPadding });

        RectInt32 dragRectangleRight{};
        dragRectangleRight.X = static_cast<int32_t>((LeftPaddingColumn().ActualWidth() + LeftDragColumn().ActualWidth() + ContentColumn().ActualWidth()) * scale);
        dragRectangleRight.Y = 0;
        dragRectangleRight.Height = appTitleBarHeight;
        dragRectangleRight.Width = static_cast<int32_t>((RightPaddingColumn().ActualWidth() + RightDragColumn().ActualWidth()) * scale);

        RectInt32 dragRectangleLeft{};
        dragRectangleLeft.X = 0;//static_cast<int32_t>((RightPaddingColumn().ActualWidth() + RightDragColumn().ActualWidth() + ContentColumn().ActualWidth()) * scale);
        dragRectangleLeft.Y = 0;
        dragRectangleLeft.Height = appTitleBarHeight;
        dragRectangleLeft.Width = static_cast<int32_t>(LeftDragColumn().ActualWidth() * scale);

        RectInt32 dragRectangles[2]{ dragRectangleRight, dragRectangleLeft };
        appWindow.TitleBar().SetDragRectangles(dragRectangles);
    }

    void FileSearchWindow::SetBackground(::Winnerino::DesktopTransparencyControllerType const& transparencyControllerType)
    {
        if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Acrylic && !DesktopAcrylicController::IsSupported())
        {
            // TODO: Trace "Desktop Acrylic Controller not supported on this platform"
            return;
        }
        else if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Mica && !MicaController::IsSupported())
        {
            // TODO: Trace "Mica Controller not supported on this platform"
            return;
        }

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

            /*activatedRevoker = Activated(auto_revoke, [this](IInspectable const&, WindowActivatedEventArgs const& args)
            {
                systemBackdropConfiguration.IsInputActive(WindowActivationState::Deactivated != args.WindowActivationState());
            });*/

            themeChangedRevoker = RootGrid().ActualThemeChanged(auto_revoke, [this](FrameworkElement const&, IInspectable const&)
            {
                systemBackdropConfiguration.Theme((SystemBackdropTheme)RootGrid().ActualTheme());
            });

            ResourceDictionary resources = Application::Current().Resources();
            if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Acrylic)
            {
                DesktopAcrylicController controller = DesktopAcrylicController();

                controller.TintColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.FallbackColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.TintOpacity(resources.TryLookup(box_value(L"BackdropTintOpacity")).as<double>());
                controller.LuminosityOpacity(resources.TryLookup(box_value(L"BackdropLuminosityOpacity")).as<double>());
                controller.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                controller.AddSystemBackdropTarget(supportsBackdrop);

                backdropController = std::move(controller);
            }
            else if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Mica)
            {
                MicaController controller = MicaController();

                controller.TintColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.FallbackColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.TintOpacity(resources.TryLookup(box_value(L"BackdropTintOpacity")).as<double>());
                controller.LuminosityOpacity(resources.TryLookup(box_value(L"BackdropLuminosityOpacity")).as<double>());
                controller.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                controller.AddSystemBackdropTarget(supportsBackdrop);

                backdropController = std::move(controller);
            }
        }
    }

    void FileSearchWindow::MainWindow_Closed(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::WindowEventArgs const&)
    {
        //Close();
    }

    winrt::Windows::Foundation::IAsyncAction FileSearchWindow::MatchFound(winrt::Windows::Foundation::IInspectable const& sender, winrt::hstring match)
    {
        try
        {
#if FALSE
            GetFileAttributesEx(match.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExMaxInfoLevel, )
                DispatcherQueue().TryEnqueue([this, _file = file]()
            {

            });
#else
            DispatcherQueue().TryEnqueue([this, filePath = match]()
            {
                TextBlock text{};
                text.Text(filePath);
                text.TextWrapping(TextWrapping::Wrap);
                SearchResults().Items().Append(text);
            });
#endif // _DEBUG

        }
        catch (const hresult_error& ex)
        {
            OutputDebugString((L"File not found \"" + match + L"\" : " + ex.message() + L"\n").c_str());
        }
        co_return;
    }

    void FileSearchWindow::AppWindow_Closing(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowClosingEventArgs const&)
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
