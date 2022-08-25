#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "LibraryTabView.g.h"

namespace winrt::Winnerino::implementation
{
    struct LibraryTabView : LibraryTabViewT<LibraryTabView>
    {
    public:
        LibraryTabView();
        LibraryTabView(hstring const& tag);

        winrt::Windows::Foundation::IAsyncAction Load(hstring const& tag);

    private:
        void GetFiles(hstring const& directory);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct LibraryTabView : LibraryTabViewT<LibraryTabView, implementation::LibraryTabView>
    {
    };
}
