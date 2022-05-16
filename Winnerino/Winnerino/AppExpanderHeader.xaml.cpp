#include "pch.h"
#include "AppExpanderHeader.xaml.h"
#if __has_include("AppExpanderHeader.g.cpp")
#include "AppExpanderHeader.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Data;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    AppExpanderHeader::AppExpanderHeader()
    {
        InitializeComponent();
    }

    hstring AppExpanderHeader::Glyph()
    {
        return _glyph;
    }

    void AppExpanderHeader::Glyph(hstring const& glyph)
    {
        _glyph = glyph;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Glyph" });
    }

    hstring AppExpanderHeader::Title()
    {
        return _title;
    }

    void AppExpanderHeader::Title(hstring const& title)
    {
        _title = title;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Title" });
    }

    hstring AppExpanderHeader::Subtitle()
    {
        return _subtitle;
    }

    void AppExpanderHeader::Subtitle(hstring const& subtitle)
    {
        _subtitle = subtitle;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Subtitle" });
    }

    event_token AppExpanderHeader::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
    {
        return m_propertyChanged.add(value);
    }

    void AppExpanderHeader::PropertyChanged(event_token const& token)
    {
        m_propertyChanged.remove(token);
    }
}
