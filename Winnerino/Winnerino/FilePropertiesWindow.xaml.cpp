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
