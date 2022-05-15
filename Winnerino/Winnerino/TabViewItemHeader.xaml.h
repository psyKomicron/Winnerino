#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "TabViewItemHeader.g.h"

namespace winrt::Winnerino::implementation
{
    struct TabViewItemHeader : TabViewItemHeaderT<TabViewItemHeader>
    {
        TabViewItemHeader();
        TabViewItemHeader(hstring const& text);

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return m_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            m_propertyChanged.remove(token);
        };

        hstring Text() const;
        void Text(hstring const& text);

        void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        hstring _text;
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct TabViewItemHeader : TabViewItemHeaderT<TabViewItemHeader, implementation::TabViewItemHeader>
    {
    };
}
