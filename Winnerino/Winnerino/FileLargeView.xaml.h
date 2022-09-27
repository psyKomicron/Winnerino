#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "FileLargeView.g.h"

#include <atomic>

namespace winrt::Winnerino::implementation
{
    struct FileLargeView : FileLargeViewT<FileLargeView>
    {
    public:
        FileLargeView();

        inline winrt::hstring FileName() { return _fileName; };
        inline winrt::hstring FilePath() { return _filePath; };
        inline bool IsDirectory() { return _isDirectory; };

        Windows::Foundation::IAsyncAction Initialize(hstring const& path, bool isFile);

        void UserControl_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void UserControl_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        std::atomic_bool loaded = false;
        Windows::Storage::StorageFile file = nullptr;
        winrt::hstring _fileName{};
        winrt::hstring _filePath{};
        bool _isDirectory = false;

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
