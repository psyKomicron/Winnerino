#include "pch.h"
#include "FileLargeView.xaml.h"
#if __has_include("FileLargeView.g.cpp")
#include "FileLargeView.g.cpp"
#endif

#include <regex>

using namespace std;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
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
        OverlayGrid().Visibility(Visibility::Collapsed);
    }

    void FileLargeView::UserControl_PointerPressed(IInspectable const&, PointerRoutedEventArgs const&)
    {
    }


    IAsyncAction FileLargeView::LoadFile(hstring path)
    {
        ImageProgressRing().IsIndeterminate(true);
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        try
        {
            file = co_await StorageFile::GetFileFromPathAsync(path);

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
                FileName().Text(file.Name());

                AttributesListView().Items().Append(box_value(L"Display name : " + file.DisplayName()));
                AttributesListView().Items().Append(box_value(L"File path : " + file.Path()));
                AttributesListView().Items().Append(box_value(L"Content type : " + file.ContentType()));
                AttributesListView().Items().Append(box_value(L"Display type : " + file.DisplayType()));
            });

            co_return;
        }
        catch (hresult_invalid_argument const& ex)
        {
        }
        catch (const hresult_error& ex)
        {
            OnException(ex.message(), path);
            co_return;
        }

        try
        {
            StorageFolder folder = co_await StorageFolder::GetFolderFromPathAsync(path);
            ThumbnailMode mode = ThumbnailMode::SingleItem;
            auto&& thumbnail = co_await folder.GetThumbnailAsync(mode, 200);
            co_await imageSource.SetSourceAsync(thumbnail);

            DispatcherQueue().TryEnqueue([this, name = folder.Name(), displayName = folder.DisplayName(), path = folder.Path()]()
            {
                ImageProgressRing().IsIndeterminate(false);
                FileName().Text(name);

                AttributesListView().Items().Append(box_value(L"Folder"));
                AttributesListView().Items().Append(box_value(L"Display name : " + displayName));
                AttributesListView().Items().Append(box_value(L"File path : " + path));
            });
        }
        catch (hresult_error const& ex)
        {
            OnException(ex.message(), path);
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

    inline void FileLargeView::OnException()
    {
        OnException(L"Failed to access file", L"");
    }

    inline void FileLargeView::OnException(hstring const& message, hstring const& name)
    {
        if (DispatcherQueue().HasThreadAccess())
        {
            ImageProgressRing().IsIndeterminate(false);
            FileName().Text(message);

            TextBlock textBlock{};
            textBlock.TextWrapping(TextWrapping::Wrap);
            textBlock.Text(L"File path : " + name);
            AttributesListView().Items().Append(textBlock);
        }
        else
        {
            DispatcherQueue().TryEnqueue([this, _message = message, _name = name]()
            {
                ImageProgressRing().IsIndeterminate(false);
                FileName().Text(_message);

                TextBlock textBlock{};
                textBlock.TextWrapping(TextWrapping::Wrap);
                textBlock.Text(L"File path : " + _name);
                AttributesListView().Items().Append(textBlock);
            });
        }
    }
}