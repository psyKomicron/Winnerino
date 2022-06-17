#include "pch.h"
#include "LibraryTabView.xaml.h"
#if __has_include("LibraryTabView.g.cpp")
#include "LibraryTabView.g.cpp"
#endif

#include <DirectoryEnumerator.h>

using namespace std;
using namespace ::Winnerino::Storage;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
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
#ifdef _DEBUG
        StorageLibrary documents = nullptr;
        if (tag == L"Documents")
        {
            documents = co_await StorageLibrary::GetLibraryAsync(KnownLibraryId::Documents);
        }
        else if (tag == L"Music")
        {
            documents = co_await StorageLibrary::GetLibraryAsync(KnownLibraryId::Music);
        }
        else if (tag == L"Pictures")
        {
            documents = co_await StorageLibrary::GetLibraryAsync(KnownLibraryId::Pictures);
        }
        else if (tag == L"Videos")
        {
            documents = co_await StorageLibrary::GetLibraryAsync(KnownLibraryId::Videos);
        }
        else
        {
            co_return;
        }

        DirectoryEnumerator enumerator{};
        auto&& folders = documents.Folders();
        for (auto&& folder : folders)
        {
            hstring path = folder.Path();
            unique_ptr<vector<hstring>> vect{ enumerator.Enumerate(path) };

            for (size_t i = 0; i < vect->size(); i++)
            {
                DispatcherQueue().TryEnqueue([this, path = vect->at(i)]()
                {
                    FileLargeView view{ path };
                    FilesList().Children().Append(view);
                });
            }
        }
#else
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
        for (StorageFile file : files)
        {
            DispatcherQueue().TryEnqueue([this, path = file.Path()]()
            {
                FileLargeView view{ path };
                FilesList().Children().Append(view);
            });
        }
#endif
    }
}