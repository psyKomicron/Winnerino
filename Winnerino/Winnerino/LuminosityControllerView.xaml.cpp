#include "pch.h"
#include <system_error>
#include "LuminosityControllerView.xaml.h"
#if __has_include("LuminosityControllerView.g.cpp")
#include "LuminosityControllerView.g.cpp"
#endif

using namespace std;

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Controls::Primitives;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Winnerino::Controllers;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    LuminosityControllerView::LuminosityControllerView()
    {
        InitializeComponent();
        try
        {
            controller.initializeController();
        }
        catch (HRESULT result)
        {
            /*int64_t severity = HRESULT_CODE(result);
            int64_t errCode = HRESULT_CODE(result);
            int64_t facilityCode = HRESULT_CODE(result);*/

            //MainWindow::Current().notifyUser(L"Luminosity cannot be changed on this computer: " + winrt::to_hstring(severity) + L"_" + winrt::to_hstring(errCode) + L"_" + winrt::to_hstring(facilityCode), InfoBarSeverity::Error);
            auto message = system_category().message(result);
            MainWindow::Current().notifyUser(to_hstring(message), InfoBarSeverity::Error);
        }
    }

#pragma region EventHandlers
    void LuminosityControllerView::luminositySlider_ValueChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        controller.setLuminosity(e.NewValue());
        luminosityTextBox().Text(to_hstring(e.NewValue()));
    }

    void LuminosityControllerView::luminosityTextBox_PreviewKeyDown(IInspectable const&, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == VirtualKey::Enter)
        {
            double luminosity = std::stod(luminosityTextBox().Text().c_str());
            controller.setLuminosity(luminosity);

            luminositySlider().Value(luminosity);
        }
    }
#pragma endregion

}
