#include "pch.h"
#include "LibraryTabView.xaml.h"
#if __has_include("LibraryTabView.g.cpp")
#include "LibraryTabView.g.cpp"
#endif

#include "DirectoryEnumerator.h"
#include "FileInfo.h"

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
        Load(tag);
    }

    IAsyncAction LibraryTabView::Load(hstring const& tag)
    {
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

        auto&& folders = documents.Folders();
        for (auto&& folder : folders)
        {
            hstring path = folder.Path();
            GetFiles(path);
        }

        DispatcherQueue().TryEnqueue([&]()
        {
            if (FilesList().Children().Size() == 0)
            {
                TextBlock textBlock{};
                textBlock.Style(Application::Current().Resources().Lookup(box_value(L"DisplayTextBlockStyle")).as<::Style>());
                textBlock.Opacity(Application::Current().Resources().Lookup(box_value(L"CaptionTextBlockOpacity")).as<double>());
                textBlock.Text(L"No files found...");
                textBlock.VerticalAlignment(VerticalAlignment::Center);
                textBlock.Margin(Thickness(0, 20, 0, 0));
                ControlScrollViewer().Content(textBlock);
            }
        });
    }

    void LibraryTabView::GetFiles(hstring const& directory)
    {
        DirectoryEnumerator enumerator{};
        unique_ptr<vector<FileInfo>> vect{ enumerator.GetFiles(directory) };

        if (vect)
        {
            if (DispatcherQueue().HasThreadAccess())
            {
                for (size_t i = 0; i < vect->size(); i++)
                {
                    FileLargeView view{ vect->at(i).Path() };
                    FilesList().Children().Append(view);
                }
            }
            else
            {
                for (size_t i = 0; i < vect->size(); i++)
                {
                    DispatcherQueue().TryEnqueue([this, i, _info = vect->at(i)]() mutable
                    {
                        FileLargeView view{ _info.Path() };
                        FilesList().Children().Append(view);
                    });
                }
            }
        }

        unique_ptr<vector<hstring>> dirs{ enumerator.EnumerateDirectories(directory) };
        if (dirs)
        {
            for (size_t i = 0; i < dirs->size(); i++)
            {
                GetFiles(dirs->at(i));
            }
        }
    }
}