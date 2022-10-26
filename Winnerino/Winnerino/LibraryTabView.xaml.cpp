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
using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
using namespace winrt::Microsoft::UI::Xaml::Input;

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;


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
            if (FilesList().Items().Size() == 0)
            {
                TextBlock textBlock{};
                textBlock.Style(Application::Current().Resources().Lookup(box_value(L"DisplayTextBlockStyle")).as<::Style>());
                textBlock.Opacity(Application::Current().Resources().Lookup(box_value(L"CaptionTextBlockOpacity")).as<double>());
                // I18N
                textBlock.Text(L"No files found...");
                textBlock.HorizontalAlignment(HorizontalAlignment::Center);
                textBlock.VerticalAlignment(VerticalAlignment::Center);

                TextBlock ascii{};
                ascii.Style(Application::Current().Resources().Lookup(box_value(L"DisplayTextBlockStyle")).as<::Style>());
                ascii.HorizontalAlignment(HorizontalAlignment::Center);
                ascii.VerticalAlignment(VerticalAlignment::Center);
                ascii.Text(L"¯\\_(ツ)_/¯");

                StackPanel panel{};
                panel.HorizontalAlignment(HorizontalAlignment::Center);
                panel.VerticalAlignment(VerticalAlignment::Center);
                panel.Orientation(Orientation::Vertical);
                panel.Spacing(7);

                panel.Children().Append(ascii);
                panel.Children().Append(textBlock);

                ControlScrollViewer().Content(panel);
            }
        });
    }

    void LibraryTabView::SizeSlider_ValueChanged(IInspectable const&, RangeBaseValueChangedEventArgs const& e)
    {
        FileLargeViewHeight().Value(box_value(e.NewValue()));
    }

    void LibraryTabView::FilesList_DoubleTapped(IInspectable const&, DoubleTappedRoutedEventArgs const&)
    {
        if (auto selected = FilesList().SelectedItem())
        {
            FileLargeView view = selected.as<FileLargeView>();

            if (view.IsDirectory())
            {
                FilesList().Items().Clear();
                GetFiles(view.FilePath());
            }
            else
            {
                Launcher::LaunchUriAsync(Uri(view.FilePath()));
            }
        }
    }


    IAsyncAction LibraryTabView::GetFiles(hstring directory)
    {
        DirectoryEnumerator enumerator{};

        unique_ptr<vector<FileInfo>> vect{ enumerator.GetFiles(directory) };
        if (vect.get())
        {
            for (size_t i = 0; i < vect->size(); i++)
            {
                FileLargeView view{};
                FilesList().Items().Append(view);
                
                try
                {
                    co_await view.Initialize(vect->at(i).Path(), !vect->at(i).IsDirectory());
                }
                catch (const hresult_access_denied&) 
                {
                    uint32_t indexof = 0;
                    if (FilesList().Items().IndexOf(view, indexof))
                    {
                        FilesList().Items().RemoveAt(indexof);
                    }
                }
            }
        }

        vect.reset(enumerator.GetFolders(directory));
        if (vect.get())
        {
            for (size_t i = 0; i < vect->size(); i++)
            {
                Expander expander{};
                expander.HorizontalAlignment(HorizontalAlignment::Stretch);
                TextBlock name{};
                TextBlock path{};
                StackPanel panel{};
                panel.Orientation(Orientation::Horizontal);

                /*name.Text(vect->at(i).Name());
                path.Text(vect->at(i).Path());
                panel.Children().Append(name);
                panel.Children().Append(path);*/

                AppExpanderHeader header{};
                header.Glyph(L"\ue838");
                header.Title(vect->at(i).Name());
                header.Subtitle(vect->at(i).Path());

                expander.Header(header);

                FoldersListView().Items().Append(expander);
            }
        }
    }
}
