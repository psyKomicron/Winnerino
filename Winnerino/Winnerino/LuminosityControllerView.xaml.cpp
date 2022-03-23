#include "pch.h"
#include "LuminosityControllerView.xaml.h"
#if __has_include("LuminosityControllerView.g.cpp")
#include "LuminosityControllerView.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Controls::Primitives;

using namespace Windows::Foundation;
using namespace Windows::System;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    LuminosityControllerView::LuminosityControllerView()
    {
        InitializeComponent();
    }

#pragma region EventHandlers
    void LuminosityControllerView::luminositySlider_ValueChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        luminosityTextBox().Text(to_hstring(e.NewValue()));
    }

    void LuminosityControllerView::luminosityTextBox_PreviewKeyDown(IInspectable const& sender, Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == VirtualKey::Enter)
        {
            double luminosity = std::stod(luminosityTextBox().Text().c_str());
            luminositySlider().Value(luminosity);
        }
    }
#pragma endregion

}
