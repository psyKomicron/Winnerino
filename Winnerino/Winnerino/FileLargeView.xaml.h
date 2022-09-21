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
        FileLargeView(hstring const& path, bool isFile);
        FileLargeView(winrt::Windows::Storage::StorageFile const& file);

        inline bool IsDirectory() { return _isDirectory; };
        inline winrt::hstring FilePath() { return _filePath; };

        void UserControl_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        Windows::Storage::StorageFile file = nullptr;
        bool _isDirectory = false;
        winrt::hstring _filePath{};

        winrt::Windows::Foundation::IAsyncAction LoadFile(hstring path);
        winrt::Windows::Foundation::IAsyncAction LoadFolder(hstring path);
        winrt::Windows::Foundation::IAsyncAction LoadFile(winrt::Windows::Storage::StorageFile file);
        inline void OnException();
        inline void OnException(winrt::hstring const& message, winrt::hstring const& name);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileLargeView : FileLargeViewT<FileLargeView, implementation::FileLargeView>
    {
    };
}
