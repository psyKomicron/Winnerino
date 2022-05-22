#include "pch.h"
#include "LibraryTabView.xaml.h"
#if __has_include("LibraryTabView.g.cpp")
#include "LibraryTabView.g.cpp"
#endif
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation::Collections;

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    LibraryTabView::LibraryTabView()
    {
        InitializeComponent();
    }

    LibraryTabView::LibraryTabView(hstring const& tag) : LibraryTabView()
    {
        LoadFiles(tag);
    }

    IAsyncAction LibraryTabView::LoadFiles(hstring const& tag)
    {
        StorageFolder documents = nullptr;

        if (tag == L"Documents")
        {
            documents = KnownFolders::DocumentsLibrary();
        }
        else if (tag == L"Music")
        {
            documents = KnownFolders::MusicLibrary();
        }
        else if (tag == L"Pictures")
        {
            documents = KnownFolders::PicturesLibrary();
        }
        else if (tag == L"Videos")
        {
            documents = KnownFolders::VideosLibrary();
        }
        else
        {
            co_return;
        }

        IVectorView<StorageFile> files = co_await documents.GetFilesAsync();

        for (const StorageFile& file : files)
        {
            DispatcherQueue().TryEnqueue([this, path = file.Path()]()
            {
                FileLargeView view{ path };
                FilesList().Children().Append(view);
            });
        }
    }
}