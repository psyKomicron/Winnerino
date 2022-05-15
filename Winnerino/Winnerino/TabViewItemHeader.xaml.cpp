#include "pch.h"
#include "TabViewItemHeader.xaml.h"
#if __has_include("TabViewItemHeader.g.cpp")
#include "TabViewItemHeader.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    TabViewItemHeader::TabViewItemHeader()
    {
        InitializeComponent();
    }

    TabViewItemHeader::TabViewItemHeader(hstring const& text)
    {
        _text = text;
        InitializeComponent();
    }

    hstring TabViewItemHeader::Text() const
    {
        return _text;
    }

    void TabViewItemHeader::Text(hstring const& text)
    {
        _text = text;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Text" });
    }

    void TabViewItemHeader::Button_Click(IInspectable const&, RoutedEventArgs const& e)
    {
        ControlTextBox().Visibility(ControlTextBox().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
        ControlTextBlock().Visibility(ControlTextBlock().Visibility() == Visibility::Visible ? Visibility::Collapsed : Visibility::Visible);
    }
}