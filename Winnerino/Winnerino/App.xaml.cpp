#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "DirectorySizeCalculator.h"
#include "FilePropertiesWindow.xaml.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace winrt::Winnerino::implementation;


/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    
    InitializeComponent();
//#ifndef DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
//    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
//    {
//        auto errorMessage = e.Message();
//        auto hres = e.Exception();
//        OutputDebugString(errorMessage.c_str());
//        MainWindow::Current().NotifyUser(errorMessage, InfoBarSeverity::Error);
//        MainWindow::Current().NotifyError(hres);
//        if (IsDebuggerPresent())
//        {
//            __debugbreak();
//        }
//    });
//#endif
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    window = make<MainWindow>();
    window.Activate();

#ifdef SPAWN_TEST_WINDOW
    Window testWindow = make<FilePropertiesWindow>();
    testWindow.Activate();
#endif // _DEBUG

}
