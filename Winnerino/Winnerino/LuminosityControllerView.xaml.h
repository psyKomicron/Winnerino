#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Input.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "LuminosityControllerView.g.h"

namespace winrt::Winnerino::implementation
{
    struct LuminosityControllerView : LuminosityControllerViewT<LuminosityControllerView>
    {
    private:

        void setLuminosity(double value);

    public:
        LuminosityControllerView();

        void luminositySlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void luminosityTextBox_PreviewKeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct LuminosityControllerView : LuminosityControllerViewT<LuminosityControllerView, implementation::LuminosityControllerView>
    {
    };
}
