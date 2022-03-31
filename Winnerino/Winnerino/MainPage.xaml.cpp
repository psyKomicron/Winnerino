#include "pch.h"
#include "MainPage.xaml.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif
using namespace winrt::Windows::ApplicationModel;

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
        PackageVersion version = Package::Current().Id().Version();
        versionTextBlock().Text(to_hstring(version.Major) + L"." + to_hstring(version.Minor) + L"." + to_hstring(version.Build) + L"." + to_hstring(version.Revision));
#if defined _DEBUG
        buildTypeTextBlock().Text(L"Debug");
#else
        buildTypeTextBlock().Text(L"Release");
#endif
    }

    
}
