#include "pch.h"
#include "FileEntryView.xaml.h"
#if __has_include("FileEntryView.g.cpp")
#include "FileEntryView.g.cpp"
#endif

#include <math.h>
#include "fileapi.h"
#include "FilePropertiesWindow.xaml.h"

using namespace std;
using namespace winrt;
using namespace ::Winnerino::Storage;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Documents;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::FileProperties;


// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileEntryView::FileEntryView()
    {
        InitializeComponent();
    }

    FileEntryView::FileEntryView(hstring const& cFileName, hstring const& path, uint64_t fileSize, int64_t attributes)
    {
        loadedEventToken = Loaded({ this, &FileEntryView::OnLoaded });
        unloadedEventToken = Unloaded({ this, &FileEntryView::OnUnloaded });

        _fileName = cFileName;
        _filePath = path;
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _isDirectory = true;
            IInspectable dirIcon = Application::Current().Resources().TryLookup(box_value(L"DirectoryFontIconGlyph"));
            _icon = unbox_value_or(dirIcon, L"\uF142");

            InitializeComponent();
            FileTypeName().Text(L"Directory");
        }
        else
        {
            UpdateSize(fileSize);

            PCWSTR ext = PathFindExtension(_fileName.c_str());
            GetIcon(ext);

            InitializeComponent(); // we can use UI properties from now on

            FileSizeTextBlock().Text(to_hstring(_displayFileSize));
            FileSizeExtensionTextBlock().Text(_fileSizeExtension);

            InitFile(ext);

            FileSizeProgressRing().IsIndeterminate(false);
            FileSizeProgressRing().Visibility(Visibility::Collapsed);
        }

        GetAttributes(attributes);
    }

    FileEntryView::FileEntryView(hstring const&)
    {
    }

    FileEntryView::~FileEntryView()
    {
        cancellationToken.cancel();
        /*try
        {
            calculateSizeTask.wait();
        }
        catch (const hresult_error&) {}
        catch (const std::exception&) {}*/

        Loaded(loadedEventToken);
        Unloaded(unloadedEventToken);
    }

    void FileEntryView::Delete()
    {
        if (!DeleteFile(_filePath.c_str()))
        {
            MainWindow::Current().NotifyError(GetLastError(), L"Could not delete " + _fileName);
        }
        else
        {
            FileNameTextBlock().Opacity(0.5);
            // raise deleted event or make the FileTabView handle directory changes ?
        }
    }

    IAsyncAction FileEntryView::Rename(hstring const& newName, bool const& generateUniqueName)
    {
#ifdef _DEBUG
        if (generateUniqueName && PathFileExists(newName.c_str()))
        {
            hstring str = newName + L"(1)";
            uint32_t iterator = 2;
            while (PathFileExists(newName.c_str()))
            {
                str = newName + L"(" + to_hstring(iterator) + L")";
                iterator++;
            }
        }

        StorageFile thisStorageItem = co_await StorageFile::GetFileFromPathAsync(_filePath);
        StorageFolder parentFolder = co_await thisStorageItem.GetParentAsync();

        hstring newFilePath = parentFolder.Path() + L"\\" + newName;

        if (!MoveFile(_filePath.c_str(), newFilePath.c_str()))
        {
            throw hresult_error(-1, L"Could not rename " + _fileName);
        }
        else
        {
            _fileName = newName;
            _filePath = newFilePath;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"FileName" });
        }
#else
        co_return;
#endif // _DEBUG
    }

    void FileEntryView::MenuFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try
        {
            DataPackage dataPackage{};
            dataPackage.SetText(_filePath);
            Clipboard::SetContent(dataPackage);
            MainWindow::Current().NotifyUser(L"Path copied to clipboard.", InfoBarSeverity::Success);
        }
        catch (const hresult& result)
        {
            MainWindow::Current().NotifyError(result);
        }
    }

    void FileEntryView::FileSizeFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Window propertiesWindow = make<Winnerino::implementation::FilePropertiesWindow>();
        propertiesWindow.Activate();
    }

    void FileEntryView::OnLoaded(IInspectable const&, RoutedEventArgs const&)
    {
        loaded = true;
        if (_isDirectory)
        {
            if (_fileName != L"." && _fileName != L"..")
            {
                concurrency::create_task([this]()
                {
                    try
                    {
                        sizeProgressToken = calculator.Progress({ get_weak(), &FileEntryView::ProgressHandler });
                        int_fast64_t dirSize = calculator.GetSize(_filePath + L"\\", true);
                        calculator.Progress(sizeProgressToken);
                        UpdateSize(dirSize);

                        if (loaded)
                        {
                            DispatcherQueue().TryEnqueue([this]()
                            {
                                FileSizeTextBlock().Text(to_hstring(_displayFileSize));
                                FileSizeExtensionTextBlock().Text(_fileSizeExtension);

                                FileSizeProgressRing().IsIndeterminate(false);
                                FileSizeProgressRing().Visibility(Visibility::Collapsed);
                            });
                        }
                    }
                    catch (const std::exception& error)
                    {
                        OutputDebugString(to_hstring(error.what()).c_str());
                        OutputDebugString(L"\n");
                    }
                    catch (hresult_error const& error)
                    {
                        OutputDebugString(error.message().c_str());
                        OutputDebugString(L"\n");
                    }
            }, cancellationToken.get_token());
            }
        }
    }

    void FileEntryView::OnUnloaded(IInspectable const&, RoutedEventArgs const&)
    {
        loaded = false;
        calculator.Progress(sizeProgressToken);
    }

    IAsyncAction FileEntryView::ToolTip_Opened(IInspectable const&, RoutedEventArgs const&)
    {
        if (PathFileExists(_filePath.c_str()))
        {
            if (!_isDirectory)
            {
                Image image{};
                image.Height(100);
                image.Stretch(Stretch::Fill);
                BitmapImage bitmapImage{};
                image.Source(bitmapImage);
                UserControlToolTip().Content(image);

                StorageFile file = co_await StorageFile::GetFileFromPathAsync(_filePath);
                ThumbnailMode thumbnailMode = ThumbnailMode::ListView;
                
                switch (this->perceivedFileType)
                {
                    case PERCEIVED_TYPE_IMAGE:
                        thumbnailMode = ThumbnailMode::PicturesView;
                        break;
                    case PERCEIVED_TYPE_AUDIO:
                        thumbnailMode = ThumbnailMode::MusicView;
                        break;
                    case PERCEIVED_TYPE_VIDEO:
                        thumbnailMode = ThumbnailMode::VideosView;
                        break;
                    case PERCEIVED_TYPE_TEXT:
                    case PERCEIVED_TYPE_COMPRESSED:
                    case PERCEIVED_TYPE_DOCUMENT:
                        thumbnailMode = ThumbnailMode::SingleItem;
                        break;
                    case PERCEIVED_TYPE_FIRST:
                    case PERCEIVED_TYPE_UNSPECIFIED:
                    case PERCEIVED_TYPE_FOLDER:
                    case PERCEIVED_TYPE_UNKNOWN:
                    case PERCEIVED_TYPE_SYSTEM:
                    case PERCEIVED_TYPE_APPLICATION:
                    case PERCEIVED_TYPE_GAMEMEDIA:
                    case PERCEIVED_TYPE_CONTACTS:
                    default:
                        break;
                }
                StorageItemThumbnail thumbnail = co_await file.GetThumbnailAsync(thumbnailMode, 200, ThumbnailOptions::ResizeThumbnail);
                co_await bitmapImage.SetSourceAsync(thumbnail);
            }
            else
            {
                TextBlock textBlock{};
                textBlock.TextTrimming(TextTrimming::CharacterEllipsis);
                UserControlToolTip().Content(textBlock);

                try
                {
                    StorageFolder folder = co_await StorageFolder::GetFolderFromPathAsync(_filePath);
                    IVectorView<IStorageItem> children = co_await folder.GetItemsAsync();
                    wostringstream builder{};
                    if (children.Size() > 1)
                    {
                        for (auto&& storageItem : children)
                        {
                            builder << storageItem.Name().c_str() << L", ";
                        }
                        wstring text = builder.str();

                        DispatcherQueue().TryEnqueue([this, _text = text, textBlock]()
                        {
                            textBlock.Text(_text);
                        });
                    }
                    else
                    {
                        DispatcherQueue().TryEnqueue([this, _text = children.GetAt(0).Name(), textBlock]()
                        {
                            textBlock.Text(_text);
                        });
                    }
                }
                catch (const hresult_error& ex)
                {
                    OutputDebugString(ex.message().c_str());
                }
            }
        }
    }


    inline hstring FileEntryView::FormatSize(double* size)
    {
        if (*size >= 0x10000000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x10000000000);
            *size /= (double)100;
            return L" Tb";
        }
        if (*size >= 0x40000000)
        {
            *size *= (double)100;
            *size = round(*size / 0x40000000);
            *size /= (double)100;
            return L" Gb";
        }
        if (*size >= 0x100000)
        {
            *size *= (double)100;
            *size = round(*size / 0x100000);
            *size /= (double)100;
            return L" Mb";
        }
        if (*size >= 0x400)
        {
            *size *= (double)100;
            *size = round(*size / 0x400);
            *size /= (double)100;
            return L" Kb";
        }
        return L" b";
    }

    void FileEntryView::GetAttributes(int64_t attributes)
    {
        ResourceDictionary resources = Application::Current().Resources();
        if (attributes & FILE_ATTRIBUTE_ARCHIVE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Archive"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_ARCHIVE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_COMPRESSED)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Compressed"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_COMPRESSED_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_DEVICE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Device"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_DEVICE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        // already used directory attribute
        if (attributes & FILE_ATTRIBUTE_ENCRYPTED)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Encrypted"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_ENCRYPTED_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_HIDDEN)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Hidden"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_HIDDEN_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
#if false
        if (attributes & FILE_ATTRIBUTE_INTEGRITY_STREAM)
        {
        }
        if (attributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
        {
        }
        if (attributes & FILE_ATTRIBUTE_NO_SCRUB_DATA)
        {
        }
#endif
        if (attributes & FILE_ATTRIBUTE_OFFLINE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Offline"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_OFFLINE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_READONLY)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Read-only"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_READONLY_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Recalled on access"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_RECALL_ON_OPEN)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Recalled on open"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_RECALL_ON_OPEN_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
#if false
        if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
        }
        if (attributes & FILE_ATTRIBUTE_SPARSE_FILE)
        {
        }
#endif
        if (attributes & FILE_ATTRIBUTE_SYSTEM)
        {
            _isSystem = true;
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"System"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_SYSTEM_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_TEMPORARY)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Temporary"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_TEMPORARY_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        if (attributes & FILE_ATTRIBUTE_VIRTUAL)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Virtual"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_VIRTUAL_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
    }

    void FileEntryView::GetIcon(PCWSTR const& ext)
    {
        hstring extension = to_hstring(ext);
        if (extension == L".pdf")
        {
            _icon = L"\uea90";
        }
        else if (extension == L".gif")
        {
            _icon = L"\uf4a9";
        }
        else if (ext != NULL)
        {
            PERCEIVED perceivedType;
            PERCEIVEDFLAG perceivedFlag;
            HRESULT result = AssocGetPerceivedType(ext, &perceivedType, &perceivedFlag, NULL);
            if (result == S_OK)
            {
                hstring key{};
                switch (perceivedType)
                {
                    /*case PERCEIVED_TYPE_CUSTOM:
                        break;
                    case PERCEIVED_TYPE_UNSPECIFIED:
                        break;
                    case PERCEIVED_TYPE_FOLDER:
                        break;
                    case PERCEIVED_TYPE_UNKNOWN:
                        break;*/
                    case PERCEIVED_TYPE_TEXT:
                        break;
                    case PERCEIVED_TYPE_IMAGE:
                        key = L"\ue8b9";
                        break;
                    case PERCEIVED_TYPE_AUDIO:
                        key = L"\ue8d6";
                        break;
                    case PERCEIVED_TYPE_VIDEO:
                        key = L"\ue714";
                        break;
                    case PERCEIVED_TYPE_COMPRESSED:
                        key = L"\uf012";
                        break;
                    case PERCEIVED_TYPE_DOCUMENT:
                        key = L"\ue8a5";
                        break;
                    case PERCEIVED_TYPE_SYSTEM:
                        key = L"\ue770";
                        break;
                    case PERCEIVED_TYPE_APPLICATION:
                        key = L"\ued35";
                        break;
                    case PERCEIVED_TYPE_GAMEMEDIA:
                        key = L"\ue7fc";
                        break;
                    case PERCEIVED_TYPE_CONTACTS:
                        key = L"\ue77b";
                        break;
                }
                _icon = key;
            }
            else
            {
                _icon = L"\uF142";
            }
        }
    }

    void FileEntryView::InitFile(PCWSTR ext)
    {
        _isDangerous = AssocIsDangerous(ext);

        DWORD bufferSize = 0;
        ASSOCSTR assocation = ASSOCSTR::ASSOCSTR_FRIENDLYAPPNAME;
        HRESULT result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", NULL, &bufferSize);
        if (result == S_FALSE)
        {
            LPWSTR str = new WCHAR[bufferSize];
            result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", str, &bufferSize);
            if (result == S_OK)
            {
                _opensWith = L"Opens with " + to_hstring(str);
            }
            delete[] str;
        }

        assocation = ASSOCSTR::ASSOCSTR_FRIENDLYDOCNAME;
        result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", nullptr, &bufferSize);
        if (result == S_FALSE)
        {
            LPWSTR string = new WCHAR[bufferSize];
            result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", string, &bufferSize);
            if (result == S_OK)
            {
                hstring docFriendlyName = to_hstring(string);
                FileTypeName().Text(docFriendlyName);
            }
            delete[] string;
        }
        else
        {
            FileTypeName().Text(L"");
        }

        PERCEIVED perceived{};
        PERCEIVEDFLAG perceivedFlag{};
        PWSTR perceivedType = nullptr;
        if (AssocGetPerceivedType(ext, &perceived, &perceivedFlag, &perceivedType) == S_OK)
        {
            _perceivedType = to_hstring(perceivedType);
        }
    }

    void FileEntryView::ProgressHandler(IInspectable const&, IReference<uint_fast64_t> const& args)
    {
        uint_fast64_t newSize = args.as<uint_fast64_t>();
        fileSize += newSize;
        double displayFileSize = static_cast<double>(fileSize);
        hstring c_ext = FormatSize(&displayFileSize);
        
#if FALSE
        if (loaded && displayFileSize != _displayFileSize)
        {
            _displayFileSize = displayFileSize;
            DispatcherQueue().TryEnqueue([this, c_newSize = displayFileSize, c_ext]()
            {
                if (FileSizeExtensionTextBlock() && FileSizeTextBlock())
                {
                    FileSizeExtensionTextBlock().Text(c_ext);
                    FileSizeTextBlock().Text(to_hstring(c_newSize));
                }
            });
        }
#else
        if (loaded && (displayFileSize - _displayFileSize) > 0.1)
        {
            _displayFileSize = displayFileSize;
            DispatcherQueue().TryEnqueue([this, c_newSize = displayFileSize, c_ext]()
            {
                FileSizeExtensionTextBlock().Text(c_ext);
                FileSizeTextBlock().Text(to_hstring(c_newSize));
            });
        }
#endif
    }

    inline void FileEntryView::UpdateSize(uint_fast64_t const& size)
    {
        this->fileSize = size;
        _displayFileSize = static_cast<double>(size);
        _fileSizeExtension = FormatSize(&_displayFileSize);
    }
}
