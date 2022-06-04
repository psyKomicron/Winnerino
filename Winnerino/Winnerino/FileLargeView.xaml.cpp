#include "pch.h"
#include "FileLargeView.xaml.h"
#if __has_include("FileLargeView.g.cpp")
#include "FileLargeView.g.cpp"
#endif

#include <regex>

using namespace std;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::FileProperties;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileLargeView::FileLargeView()
    {
        InitializeComponent();
    }

    FileLargeView::FileLargeView(hstring const& path) : FileLargeView()
    {
        LoadFile(path);
    }

    FileLargeView::FileLargeView(StorageFile const& file, IInspectable const&)
    {
        LoadFile(file);
    }

    void FileLargeView::Grid_PointerEntered(IInspectable const&, PointerRoutedEventArgs const&)
    {
        OverlayGrid().Visibility(Visibility::Visible);
        OverlayGrid().Opacity(1);
    }

    void FileLargeView::Grid_PointerExited(IInspectable const&, PointerRoutedEventArgs const&)
    {
        OverlayGrid().Opacity(0);
    }

    void FileLargeView::UserControl_PointerPressed(IInspectable const&, PointerRoutedEventArgs const&)
    {
    }


    IAsyncAction FileLargeView::LoadFile(hstring const& path)
    {
        ImageProgressRing().IsIndeterminate(true);
        FileName().Text(path);
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        try
        {
            StorageFile file = co_await StorageFile::GetFileFromPathAsync(path);
            ThumbnailMode mode = ThumbnailMode::ListView;

            wregex audioRe = wregex{ L"^audio" };
            wregex videoRe = wregex{ L"^video" };
            wregex imageRe = wregex{ L"^image" };

            hstring contentType = file.ContentType();
            if (regex_search(contentType.c_str(), audioRe))
            {
                mode = ThumbnailMode::MusicView;
            }
            else if (regex_search(contentType.c_str(), videoRe))
            {
                mode = ThumbnailMode::VideosView;
            }
            else if (regex_search(contentType.c_str(), imageRe))
            {
                mode = ThumbnailMode::PicturesView;
            }

            co_await imageSource.SetSourceAsync(co_await file.GetThumbnailAsync(mode, 200));
            DispatcherQueue().TryEnqueue([this]()
            {
                ImageProgressRing().IsIndeterminate(false);
            });
        }
        catch (const hresult_error& ex)
        {
            OutputDebugString((ex.message() + L"\n").c_str());
        }
    }

    IAsyncAction FileLargeView::LoadFile(StorageFile file)
    {
        ImageProgressRing().IsIndeterminate(true);
        FileName().Text(file.Path());
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        try
        {
            ThumbnailMode mode = ThumbnailMode::ListView;

            wregex audioRe = wregex{ L"^audio" };
            wregex videoRe = wregex{ L"^video" };
            wregex imageRe = wregex{ L"^image" };

            hstring contentType = file.ContentType();
            if (regex_search(contentType.c_str(), audioRe))
            {
                mode = ThumbnailMode::MusicView;
            }
            else if (regex_search(contentType.c_str(), videoRe))
            {
                mode = ThumbnailMode::VideosView;
            }
            else if (regex_search(contentType.c_str(), imageRe))
            {
                mode = ThumbnailMode::PicturesView;
            }

            co_await imageSource.SetSourceAsync(co_await file.GetThumbnailAsync(mode, 200));
            DispatcherQueue().TryEnqueue([this]()
            {
                ImageProgressRing().IsIndeterminate(false);
            });
        }
        catch (const hresult_error& ex)
        {
            OutputDebugString((ex.message() + L"\n").c_str());
        }
    }
}