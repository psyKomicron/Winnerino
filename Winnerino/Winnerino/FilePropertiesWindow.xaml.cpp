#include "pch.h"
#include "FilePropertiesWindow.xaml.h"
#if __has_include("FilePropertiesWindow.g.cpp")
#include "FilePropertiesWindow.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Windows::Graphics;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FilePropertiesWindow::FilePropertiesWindow()
    {
        InitializeComponent();
        InitWindow();
    }

    FilePropertiesWindow::~FilePropertiesWindow()
    {
        appWindow.Destroy();
    }

    void FilePropertiesWindow::InitWindow()
    {
#pragma region settings
#if FALSE
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
#endif // FALSE

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
        appWindow.Title(L"Properties");
        if (appWindow != nullptr)
        {
            RectInt32 rect{};
            rect.Height = 500;
            rect.Width = 200;
            rect.X = 100;
            rect.Y = 100;
            appWindow.MoveAndResize(rect);

            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                appWindow.TitleBar().ExtendsContentIntoTitleBar(false);

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
            else // hide title bar
            {
                /*uint32_t index = 0;
                if (contentGrid().Children().IndexOf(titleBarGrid(), index))
                {
                    contentGrid().Children().RemoveAt(index);
                }
                contentGrid().RowDefinitions().RemoveAt(0);*/
            }
        }
    }
}
