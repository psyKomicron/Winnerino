#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
    }

    void MainPage::OnLoaded(IInspectable const&, RoutedEventArgs const&)
    {
    }

    void MainPage::Button_Click(IInspectable const&, RoutedEventArgs const&)
    {
        MainWindow::Current().GoBack();
    }
}


