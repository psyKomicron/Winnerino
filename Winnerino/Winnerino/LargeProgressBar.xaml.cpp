#include "pch.h"
#include "LargeProgressBar.xaml.h"
#if __has_include("LargeProgressBar.g.cpp")
#include "LargeProgressBar.g.cpp"
#endif

#include "math.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    LargeProgressBar::LargeProgressBar()
    {
        InitializeComponent();
    }

    void LargeProgressBar::Value(double const& value)
    {
        _value = value;
        SetWidth();

        e_propertyChanged(*this, PropertyChangedEventArgs{ L"Percentage" });
    }

    double LargeProgressBar::Value() const
    {
        return _value;
    }

    void LargeProgressBar::Maximum(double const& value)
    {
        _maximum = value;
        SetWidth();

        e_propertyChanged(*this, PropertyChangedEventArgs{ L"Percentage" });
    }

    double LargeProgressBar::Maximum() const
    {
        return _maximum;
    }

    double LargeProgressBar::Percentage() const
    {
        return _maximum == 0 ? 0 : round((_value / _maximum) * 100.0);
    }

    void LargeProgressBar::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        SetWidth();
    }

    void LargeProgressBar::Grid_SizeChanged(IInspectable const&, SizeChangedEventArgs const&)
    {
        SetWidth();
    }

    void LargeProgressBar::SetWidth()
    {
        if (_maximum == 0)
        {
            ProgressBorder().Width(0);
        }
        else
        {
            double barWidth = (_value / _maximum) * BackgroundBorder().ActualWidth();
            ProgressBorder().Width(barWidth);
        }
    }
}