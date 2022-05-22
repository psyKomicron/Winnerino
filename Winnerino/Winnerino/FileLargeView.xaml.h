﻿#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "FileLargeView.g.h"

namespace winrt::Winnerino::implementation
{
    struct FileLargeView : FileLargeViewT<FileLargeView>
    {
    public:
        FileLargeView();
        FileLargeView(hstring const& path);

        void Grid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        //Windows::Storage::StorageFolder folder;

        winrt::Windows::Foundation::IAsyncAction LoadFile(hstring const& path);
    public:
        void UserControl_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileLargeView : FileLargeViewT<FileLargeView, implementation::FileLargeView>
    {
    };
}