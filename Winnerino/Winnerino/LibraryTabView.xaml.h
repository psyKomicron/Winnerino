#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "LibraryTabView.g.h"

namespace winrt::Winnerino::implementation
{
    struct LibraryTabView : LibraryTabViewT<LibraryTabView>
    {
        LibraryTabView();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct LibraryTabView : LibraryTabViewT<LibraryTabView, implementation::LibraryTabView>
    {
    };
}
