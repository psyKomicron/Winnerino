#pragma once

#include "WidgetsPage.g.h"

namespace winrt::Winnerino::implementation
{
    struct WidgetsPage : WidgetsPageT<WidgetsPage>
    {
        WidgetsPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct WidgetsPage : WidgetsPageT<WidgetsPage, implementation::WidgetsPage>
    {
    };
}
