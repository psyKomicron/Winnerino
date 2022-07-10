#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "AppExpanderHeader.g.h"

namespace winrt::Winnerino::implementation
{
    struct AppExpanderHeader : AppExpanderHeaderT<AppExpanderHeader>
    {
        AppExpanderHeader();

        hstring Glyph();
        void Glyph(hstring const& glyph);
        hstring Title();
        void Title(hstring const& title);
        hstring Subtitle();
        void Subtitle(hstring const& subtitle);

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value);
        void PropertyChanged(event_token const& token);

    private:
        hstring _glyph;
        hstring _title;
        hstring _subtitle;
        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct AppExpanderHeader : AppExpanderHeaderT<AppExpanderHeader, implementation::AppExpanderHeader>
    {
    };
}
