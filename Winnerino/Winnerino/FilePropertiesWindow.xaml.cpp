#include "pch.h"
#include "FilePropertiesWindow.xaml.h"
#if __has_include("FilePropertiesWindow.g.cpp")
#include "FilePropertiesWindow.g.cpp"
#endif

#define INVALIDATE_VIEW 0
#define RESET_TITLE_BAR 0

using namespace winrt;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Graphics;

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
        appWindow.Changed(appWindowChangedEventToken);
        appWindow.Destroy();
    }

    void FilePropertiesWindow::TitleBarGrid_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        if (AppWindowTitleBar::IsCustomizationSupported() && appWindow.TitleBar().ExtendsContentIntoTitleBar())
        {
            //SetDragRectangles();
        }
    }

    void FilePropertiesWindow::LocalTabView_TabCloseRequested(TabView const& sender, TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t indexOf;
        if (sender.TabItems().IndexOf(args.Item(), indexOf))
        {
            sender.TabItems().RemoveAt(indexOf); // why do we need to do this ? Can't the TabView do that itself ?
        }
    }

    void FilePropertiesWindow::LocalTabView_AddTabButtonClick(TabView const& sender, IInspectable const&)
    {
        TabViewItem item{};
        item.Header(box_value(L"New tab"));
        sender.TabItems().Append(item);
    }

    void FilePropertiesWindow::TabStripFooter_SizeChanged(IInspectable const&, SizeChangedEventArgs const&)
    {
        OutputDebugString(L"Size changed\n");
        SetDragRectangles();
    }


    void FilePropertiesWindow::OnWindowChanged(winrt::Microsoft::UI::Windowing::AppWindow const&, winrt::Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args)
    {
        /*if (args.DidSizeChange())
        {
            SetDragRectangles();
        }*/
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
        appWindowChangedEventToken = appWindow.Changed({ this, &FilePropertiesWindow::OnWindowChanged });
        appWindow.Title(L"Properties");
        if (appWindow != nullptr)
        {
            RectInt32 rect{};
            rect.Height = 400;
            rect.Width = 1200;
            rect.X = 100;
            rect.Y = 100;
            appWindow.MoveAndResize(rect);

            if (AppWindowTitleBar::IsCustomizationSupported())
            {
                appWindow.TitleBar().ExtendsContentIntoTitleBar(true);

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
        }
    }

    void FilePropertiesWindow::SetDragRectangles()
    {
#if RESET_TITLE_BAR
        appWindow.TitleBar().ResetToDefault();
        appWindow.TitleBar().ExtendsContentIntoTitleBar(true);
#endif // DEBUG

        HWND windowHandle = GetWindowFromWindowId(appWindow.Id());
        UINT windowDpi = GetDpiForWindow(windowHandle);

        double dpi = (windowDpi * static_cast<double>(100) + (static_cast<double>(96) / 2)) / 96;
        double scale = dpi / static_cast<double>(100);

        int32_t appTitleBarHeight = static_cast<int32_t>(48 * scale);

        double leftPadding = appWindow.TitleBar().LeftInset() / scale;
        double rightPadding = appWindow.TitleBar().RightInset() / scale;

        RightPaddingColumn().Width(GridLength{ rightPadding });
        LeftPaddingColumn().Width(GridLength{ leftPadding });

        RectInt32 dragRectangleRight{};
        dragRectangleRight.X = static_cast<int32_t>((appWindow.ClientSize().Width - RightDragColumn().ActualWidth() - RightPaddingColumn().ActualWidth()) * scale);
        dragRectangleRight.Y = 0;
        dragRectangleRight.Height = appTitleBarHeight;
        dragRectangleRight.Width = static_cast<int32_t>(RightDragColumn().ActualWidth() * scale);

        RectInt32 dragRectangleLeft{};
        dragRectangleLeft.X = static_cast<int32_t>(LeftPaddingColumn().ActualWidth() * scale);
        dragRectangleLeft.Y = 0;
        dragRectangleLeft.Height = appTitleBarHeight;
        dragRectangleLeft.Width = static_cast<int32_t>(LeftDragColumn().ActualWidth() * scale);

        if (dragRectangleLeft.Width < 0)
        {
            dragRectangleLeft.Width = 0;
        }
        if (dragRectangleRight.Width < 0)
        {
            dragRectangleRight.Width = 0;
        }

        RectInt32 dragRectangles[2]{ dragRectangleRight, dragRectangleLeft };
        appWindow.TitleBar().SetDragRectangles(dragRectangles);

#if INVALIDATE_VIEW
        TitleBarGrid().InvalidateMeasure();
        TitleBarGrid().InvalidateArrange();
        LocalTabView().IsHitTestVisible(true);
        //LocalTabView().InvalidateViewport();  
#endif // INVALIDATE_VIEW

        OutputDebugString((L"X = " + to_hstring(dragRectangleRight.X) + L", Y = " + to_hstring(dragRectangleRight.Y) + L" | "
                          + L"Width = " + to_hstring(dragRectangleRight.Width) + L", Height = " + to_hstring(dragRectangleRight.Height) + L"\n").c_str());

        FooterWidth().Text(to_hstring(dragRectangleRight.Width));
        FooterX().Text(to_hstring(dragRectangleRight.X));
        WindowWidthTextBlock().Text(to_hstring(appWindow.ClientSize().Width));
        RightPaddingTextBlock().Text(to_hstring(RightPaddingColumn().ActualWidth()));
    }
}