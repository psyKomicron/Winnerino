#include "pch.h"
#include "FilePropertiesWindow.xaml.h"
#if __has_include("FilePropertiesWindow.g.cpp")
#include "FilePropertiesWindow.g.cpp"
#endif

#include "FileInfo.h"
#include "Helper.h"
#include "DirectoryEnumerator.h"
#include <DirectorySizeCalculator.h>

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
    }

    IAsyncAction FilePropertiesWindow::LoadFile(winrt::hstring file)
    {
        try
        {
            auto task = StorageFile::GetFileFromPathAsync(file);
            DirectorySizeCalculator calc{};

            StorageFile storageFile = co_await task;

            DispatcherQueue().TryEnqueue([this, name = storageFile.Name(), path = storageFile.Path(), displayType = storageFile.DisplayType()]()
            {
                FileNameTextBox().Text(name);
                FilePathTextBlock().Text(path);
                FileTypeTextBlock().Text(displayType);
            });


            auto&& props = co_await storageFile.GetBasicPropertiesAsync();
            //auto extraProps = co_await props.RetrievePropertiesAsync();

            DispatcherQueue().TryEnqueue([this, dateModified = props.DateModified(), dateCreated = props.ItemDate(), size = props.Size()]()
            {
                // TODO: User defined date formatting
                Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter
                {
                    L"{hour.integer}:{minute.integer(2)}:{second.integer(2)} {month.integer(2)}/{day.integer(2)}/{year.full}"
                };

                ModifiedDateTextBlock().Text(formatter.Format(dateModified));
                CreatedDateTextBlock().Text(formatter.Format(dateCreated));


                double formattedSize = static_cast<double>(size);
                hstring ext = ::Winnerino::format_size(&formattedSize, 2);
                if (size < 1024)
                {
                    SizeCompressedTextBlock().Text(to_hstring(formattedSize) + ext);
                }
                else
                {
                    winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceLoader loader{};
                    SizeCompressedTextBlock().Text(to_hstring(formattedSize) + ext + L" - " + to_hstring(size) + L" " + loader.GetString(L"BytesFileSizeExtension"));
                }
            });
        }
        catch (const hresult_access_denied&)
        {
            DispatcherQueue().TryEnqueue([this, c_file = file]()
            {
                OnAccessDenied(c_file);
            });
        }
    }

    IAsyncAction FilePropertiesWindow::LoadFolder(winrt::hstring folder)
    {
        try
        {
            auto task = StorageFolder::GetFolderFromPathAsync(folder);
            DirectorySizeCalculator calc{};
            uint64_t size = calc.GetSize(folder.ends_with(L'\\') ? folder : folder + L"\\", true);

            StorageFolder storageFolder = co_await task;

            {
                DirectoryEnumerator enumerator = DirectoryEnumerator(false);
                uint32_t fileChidren = 0, folderChildren = 0;
                std::unique_ptr<std::vector<hstring>> files{ enumerator.EnumerateFiles(folder) };
                std::unique_ptr<std::vector<hstring>> folders{ enumerator.EnumerateFolders(folder) };

                if (files.get() && folders.get())
                {
                    fileChidren = files->size();
                    folderChildren = folders->size();

                    ChildrenTextBlock().Text(to_hstring(fileChidren) + L" files, " + to_hstring(folderChildren) + L" folders.");
                }
                else
                {
                    ChildrenTextBlock().Text(L"access denied.");
                }
            }

            DispatcherQueue().TryEnqueue([this, size, name = storageFolder.Name(), path = storageFolder.Path(), displayType = storageFolder.DisplayType()]()
            {
                FileNameTextBox().Text(name);
                FilePathTextBlock().Text(path);
                FileTypeTextBlock().Text(displayType);

                double formattedSize = static_cast<double>(size);
                hstring ext = ::Winnerino::format_size(&formattedSize, 2);
                winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceLoader loader{};
                SizeCompressedTextBlock().Text(to_hstring(formattedSize) + ext + L" - " + to_hstring(size) + L" " + loader.GetString(L"BytesFileSizeExtension"));
            });


            auto&& props = co_await storageFolder.GetBasicPropertiesAsync();
            //auto extraProps = co_await props.RetrievePropertiesAsync();

            DispatcherQueue().TryEnqueue([this, dateModified = props.DateModified(), dateCreated = props.ItemDate()]()
            {
                // TODO: User defined date formatting
                Windows::Globalization::DateTimeFormatting::DateTimeFormatter formatter
                {
                    L"{hour.integer}:{minute.integer(2)}:{second.integer(2)} {month.integer(2)}/{day.integer(2)}/{year.full}"
                };

                ModifiedDateTextBlock().Text(formatter.Format(dateModified));
                CreatedDateTextBlock().Text(formatter.Format(dateCreated));
            });
        }
        catch (const hresult_access_denied&)
        {
            DispatcherQueue().TryEnqueue([this, c_folder = folder]()
            {
                OnAccessDenied(c_folder);
            });
        }
    }

    void FilePropertiesWindow::Control_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        RootGrid().RequestedTheme((ElementTheme)unbox_value_or(
            ApplicationData::Current().LocalSettings().Values().TryLookup(L"AppTheme"), static_cast<int32_t>(ElementTheme::Default))
        );

        ContentGrid().Focus(FocusState::Pointer);
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
        winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceLoader resLoader{};
        appWindow.Title(resLoader.GetString(L"FilePropertiesWindowTitle"));
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

        presenter.IsAlwaysOnTop(unbox_value_or(settings.Values().TryLookup(L"KeepOnTop"), false));
    }

    void FilePropertiesWindow::SetBackground(::Winnerino::DesktopTransparencyControllerType const& transparencyControllerType)
    {
        if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Acrylic && !DesktopAcrylicController::IsSupported())
        {
            OutputDebugString(L"FilePropertiesWindow > Desktop Acrylic Controller not supported on this platform");
            return;
        }
        else if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Mica && !MicaController::IsSupported())
        {
            OutputDebugString(L"FilePropertiesWindow > Mica Controller not supported on this platform");
            return;
        }

        auto&& supportsBackdrop = try_as<ICompositionSupportsSystemBackdrop>();
        if (supportsBackdrop)
        {
            RootGrid().Background(SolidColorBrush(Colors::Transparent()));

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


            ResourceDictionary resources = Application::Current().Resources();
            if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Acrylic)
            {
                DesktopAcrylicController controller = DesktopAcrylicController();
                
                controller.TintColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.FallbackColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.TintOpacity(
                    static_cast<float>(resources.TryLookup(box_value(L"BackdropTintOpacity")).as<double>() + 0.05)
                );
                controller.LuminosityOpacity(
                    static_cast<float>(resources.TryLookup(box_value(L"BackdropLuminosityOpacity")).as<double>() - 0.1)
                );
                controller.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                controller.AddSystemBackdropTarget(supportsBackdrop);

                backdropController = std::move(controller);
            }
            else if (transparencyControllerType == ::Winnerino::DesktopTransparencyControllerType::Mica)
            {
                MicaController controller = MicaController();

                controller.TintColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.FallbackColor(resources.TryLookup(box_value(L"SolidBackgroundFillColorBase")).as<Windows::UI::Color>());
                controller.TintOpacity(
                    static_cast<float>(resources.TryLookup(box_value(L"BackdropSecondaryTintOpacity")).as<double>())
                );
                controller.LuminosityOpacity(
                    static_cast<float>(resources.TryLookup(box_value(L"BackdropSecondaryLuminosityOpacity")).as<double>())
                );
                controller.SetSystemBackdropConfiguration(systemBackdropConfiguration);
                controller.AddSystemBackdropTarget(supportsBackdrop);

                backdropController = std::move(controller);
            }
        }
    }

    void FilePropertiesWindow::OnAccessDenied(hstring path)
    {
        static char count = 0;
        const hstring ascii[10]{ L"￣へ￣", L"＞﹏＜", L"(╯°□°）╯︵ ┻━┻", L"(►__◄)", L"ಠ﹏ಠ", L"<(＿　＿)>", L"::>_<::", L"(╯▔皿▔)╯", L"(´。＿。｀)", L"(* ￣︿￣)" };
        // I18N: Access denied

        ContentGrid().RowDefinitions().Clear();
        ContentGrid().Children().Clear();

        TextBlock block{};
        TextBlock block2{};

        block.HorizontalAlignment(HorizontalAlignment::Center);
        block.VerticalAlignment(VerticalAlignment::Center);
        block.FontSize(20);

        block2.HorizontalAlignment(HorizontalAlignment::Center);
        block2.VerticalAlignment(VerticalAlignment::Center);
        block2.TextWrapping(TextWrapping::Wrap);
        block2.Margin(Thickness(15, 0, 15, 0));


        StackPanel panel{};
        panel.Spacing(7);
        panel.Orientation(Orientation::Vertical);
        panel.HorizontalAlignment(HorizontalAlignment::Center);
        panel.VerticalAlignment(VerticalAlignment::Center);

        block.Text(ascii[count % 10]);
        block2.Text(L"Access denied (" + path + L")");

        panel.Children().Append(block);
        panel.Children().Append(block2);

        ContentGrid().Children().Append(panel);

        count++;
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