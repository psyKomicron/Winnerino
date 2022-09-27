#include "pch.h"
#include "FileLargeView.xaml.h"
#if __has_include("FileLargeView.g.cpp")
#include "FileLargeView.g.cpp"
#endif

#include <regex>
#include <shlwapi.h>

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


namespace winrt::Winnerino::implementation
{
    FileLargeView::FileLargeView()
    {
        InitializeComponent();
        loaded.store(true);
    }


    IAsyncAction FileLargeView::Initialize(hstring const& path, bool isFile)
    {
        _isDirectory = !isFile;
        _filePath = path;

        if (isFile)
        {
            co_await LoadFile(path);
        }
        else
        {
            co_await LoadFolder(path);
        }
    }

    void FileLargeView::Grid_PointerEntered(IInspectable const&, PointerRoutedEventArgs const&)
    {
    }

    void FileLargeView::Grid_PointerExited(IInspectable const&, PointerRoutedEventArgs const&)
    {
    }

    void FileLargeView::UserControl_PointerPressed(IInspectable const&, PointerRoutedEventArgs const&)
    {
    }

    void FileLargeView::UserControl_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        //loaded.store(true);
    }

    void FileLargeView::UserControl_Unloaded(IInspectable const&, RoutedEventArgs const&)
    {
        loaded.store(false);
    }


    IAsyncAction FileLargeView::LoadFile(hstring path)
    {
        //auto lifetime = get_strong();

        ImageProgressRing().IsIndeterminate(true);
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        file = co_await StorageFile::GetFileFromPathAsync(path);

        PERCEIVEDFLAG perceivedFlag{};
        PERCEIVED perceivedFileType{};
        PCWSTR ext = file.FileType().c_str();
        if (FAILED(AssocGetPerceivedType(ext, &perceivedFileType, &perceivedFlag, NULL)))
        {
            perceivedFileType = PERCEIVED::PERCEIVED_TYPE_UNKNOWN;
        }

        if (perceivedFileType == PERCEIVED::PERCEIVED_TYPE_IMAGE)
        {
            imageSource.UriSource(Uri(path));
        }
        else
        {
            StorageItemThumbnail thumbnail = co_await file.GetThumbnailAsync(ThumbnailMode::SingleItem, 300, ThumbnailOptions::UseCurrentScale);
            co_await imageSource.SetSourceAsync(thumbnail);
        }

        if (loaded.load())
        {
            DispatcherQueue().TryEnqueue([this, c_perceivedFileType = perceivedFileType]()
            {
                ImageProgressRing().IsIndeterminate(false);
                FileNameTextBlock().Text(file.Name());

                AttributesListView().Items().Append(box_value(L"Display name : " + file.DisplayName()));
                AttributesListView().Items().Append(box_value(L"File path : " + file.Path()));
                AttributesListView().Items().Append(box_value(L"Content type : " + file.ContentType()));
                AttributesListView().Items().Append(box_value(L"Display type : " + file.DisplayType()));

                switch (c_perceivedFileType)
                {
                    case PERCEIVED::PERCEIVED_TYPE_IMAGE:
                        FileTypeFontIcon().Glyph(L"\ue8b9");
                        break;
                    case PERCEIVED::PERCEIVED_TYPE_VIDEO:
                        FileTypeFontIcon().Glyph(L"\ue714");
                        break;
                    case PERCEIVED::PERCEIVED_TYPE_AUDIO:
                        FileTypeFontIcon().Glyph(L"\ue8d6");
                        break;
                    default:
                        FileTypeFontIcon().Glyph(L"\ue8a5");
                        break;
                }
            });
        }
    }

    IAsyncAction FileLargeView::LoadFolder(hstring path)
    {
        ImageProgressRing().IsIndeterminate(true);
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        StorageFolder folder = co_await StorageFolder::GetFolderFromPathAsync(path);
        
        auto&& thumbnail = co_await folder.GetThumbnailAsync(ThumbnailMode::SingleItem, 300, ThumbnailOptions::UseCurrentScale);
        co_await imageSource.SetSourceAsync(thumbnail);

        if (loaded.load())
        {
            DispatcherQueue().TryEnqueue([this, name = folder.Name(), displayName = folder.DisplayName(), path = folder.Path()]()
            {
                ImageProgressRing().IsIndeterminate(false);
                FileNameTextBlock().Text(name);

                AttributesListView().Items().Append(box_value(L"Folder"));
                AttributesListView().Items().Append(box_value(L"Display name : " + displayName));
                AttributesListView().Items().Append(box_value(L"File path : " + path));
            });
        }
    }

    IAsyncAction FileLargeView::LoadFile(StorageFile storageFile)
    {
        ImageProgressRing().IsIndeterminate(true);
        FileNameTextBlock().Text(storageFile.Path());
        BitmapImage imageSource{};
        Thumbnail().Source(imageSource);

        try
        {
            ThumbnailMode mode = ThumbnailMode::ListView;

            wregex audioRe = wregex{ L"^audio" };
            wregex videoRe = wregex{ L"^video" };
            wregex imageRe = wregex{ L"^image" };

            hstring contentType = storageFile.ContentType();
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

            file = storageFile;

            DispatcherQueue().TryEnqueue([this]()
            {
                ImageProgressRing().IsIndeterminate(false);
                FileNameTextBlock().Text(file.Name());

                AttributesListView().Items().Append(box_value(L"Display name : " + file.DisplayName()));
                AttributesListView().Items().Append(box_value(L"File path : " + file.Path()));
                AttributesListView().Items().Append(box_value(L"Content type : " + file.ContentType()));
                AttributesListView().Items().Append(box_value(L"Display type : " + file.DisplayType()));
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
            FileNameTextBlock().Text(message);

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
                FileNameTextBlock().Text(_message);

                TextBlock textBlock{};
                textBlock.TextWrapping(TextWrapping::Wrap);
                textBlock.Text(L"File path : " + _name);
                AttributesListView().Items().Append(textBlock);
            });
        }
    }
}
