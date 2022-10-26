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
        void SizeSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);

    private:
        winrt::Windows::Foundation::IAsyncAction GetFiles(hstring directory);
    public:
        void FilesList_DoubleTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct LibraryTabView : LibraryTabViewT<LibraryTabView, implementation::LibraryTabView>
    {
    };
}
