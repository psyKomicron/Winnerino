#include "pch.h"
#include "FileEntryView.xaml.h"
#if __has_include("FileEntryView.g.cpp")
#include "FileEntryView.g.cpp"
#endif

#include <math.h>
#include "fileapi.h"
#include "FilePropertiesWindow.xaml.h"
#include "Helper.h"

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
using namespace winrt::Windows::Media::Core;
using namespace winrt::Windows::Media::Playback;
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

    FileEntryView::FileEntryView(hstring const& cFileName, hstring const& path, uint64_t fileSize, int64_t attributes) : FileEntryView()
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
            //FileTypeName().Text(L"Directory");
        }
        else
        {
            UpdateSize(fileSize);

            PCWSTR ext = PathFindExtension(_fileName.c_str());
            _fileExtension = to_hstring(ext);
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


    uint64_t FileEntryView::FileBytes() const
    {
        return fileSize;
    }

    hstring FileEntryView::FileName() const
    {
        return _fileName;
    }

    hstring FileEntryView::FilePath() const
    {
        return _filePath;
    }

    hstring FileEntryView::FileIcon() const
    {
        return _icon;
    }

    hstring FileEntryView::FileSize() const
    {
        return to_hstring(_displayFileSize) + _fileSizeExtension;
    }

    hstring FileEntryView::FileExtension() const
    {
        return _fileExtension;
    }

    bool FileEntryView::IsDirectory() const
    {
        return _isDirectory;
    }

    bool FileEntryView::IsSystem() const
    {
        return _isSystem;
    }

    hstring FileEntryView::OpensWith() const
    {
        return _opensWith;
    }

    hstring FileEntryView::PerceivedType() const
    {
        return _perceivedType;
    }

    Windows::Foundation::DateTime FileEntryView::LastWrite() const
    {
        return _lastWrite;
    }

    void FileEntryView::LastWrite(Windows::Foundation::DateTime const& value)
    {
        _lastWrite = value;
    }

    bool FileEntryView::IsFileDangerous() const
    {
        return _isDangerous;
    }

    bool FileEntryView::ShowFilePath() const
    {
        return _showFilePath;
    }

    void FileEntryView::ShowFilePath(bool value)
    {
        _showFilePath = value;
        e_propertyChanged(*this, PropertyChangedEventArgs{ L"ShowFilePath" });
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
            e_deleted(*this, IInspectable());
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
                sizeProgressToken = calculator.Progress({ get_weak(), &FileEntryView::ProgressHandler });
                concurrency::create_task([this]()
                {
                    try
                    {
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
            if (IsDirectory())
            {
                try
                {
                    StorageFolder folder = co_await StorageFolder::GetFolderFromPathAsync(_filePath);
                    IVectorView<IStorageItem> children = co_await folder.GetItemsAsync();
                    wostringstream builder{};
                    if (children.Size() > 1)
                    {
                        builder << children.GetAt(0).Name().c_str();
                        for (size_t i = 1; i < children.Size(); i++)
                        {
                            builder << L", " << children.GetAt(i).Name().c_str();
                        }

                        DispatcherQueue().TryEnqueue([this, _text = builder.str()]()
                        {
                            TextBlock textBlock{};
                            textBlock.TextTrimming(TextTrimming::CharacterEllipsis);
                            textBlock.Text(_text);

                            UserControlToolTip().Content(textBlock);
                        });
                    }
                    else
                    {
                        DispatcherQueue().TryEnqueue([this, _text = children.GetAt(0).Name()]()
                        {
                            TextBlock textBlock{};
                            textBlock.TextTrimming(TextTrimming::CharacterEllipsis);
                            textBlock.Text(_text);

                            UserControlToolTip().Content(textBlock);
                        });
                    }
                }
                catch (const hresult_error& ex)
                {
                    DispatcherQueue().TryEnqueue([this, message = ex.message()]()
                    {
                        TextBlock textBlock{};
                        textBlock.TextTrimming(TextTrimming::CharacterEllipsis);
                        textBlock.Text(message);

                        UserControlToolTip().Content(textBlock);
                    });
                    OutputDebugString(ex.message().c_str());
                }
            }
            else
            {
                Image image{};
                image.Stretch(Stretch::Uniform);
                BitmapImage bitmapImage{};
                image.Source(bitmapImage);

                Viewbox viewBox{};

                viewBox.HorizontalAlignment(HorizontalAlignment::Center);
                viewBox.VerticalAlignment(VerticalAlignment::Stretch);
                viewBox.Child(image);

                UserControlToolTip().Content(viewBox);

                StorageFile file = co_await StorageFile::GetFileFromPathAsync(_filePath);
                ThumbnailMode thumbnailMode = ThumbnailMode::ListView;

                switch (perceivedFileType)
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
                    case PERCEIVED_TYPE_DOCUMENT:
                        thumbnailMode = ThumbnailMode::DocumentsView;
                        break;
                    case PERCEIVED_TYPE_COMPRESSED:
                    case PERCEIVED_TYPE_FIRST:
                    case PERCEIVED_TYPE_UNSPECIFIED:
                    case PERCEIVED_TYPE_FOLDER:
                    case PERCEIVED_TYPE_UNKNOWN:
                    case PERCEIVED_TYPE_SYSTEM:
                    case PERCEIVED_TYPE_APPLICATION:
                    case PERCEIVED_TYPE_GAMEMEDIA:
                    case PERCEIVED_TYPE_CONTACTS:
                    default:
                        thumbnailMode = ThumbnailMode::SingleItem;
                        break;
                }

                StorageItemThumbnail thumbnail = co_await file.GetThumbnailAsync(thumbnailMode, 500, ThumbnailOptions::UseCurrentScale);
                co_await bitmapImage.SetSourceAsync(thumbnail);
            }
        }
    }


    void FileEntryView::GetAttributes(int64_t _attributes)
    {
        ResourceDictionary resources = Application::Current().Resources();
        if (_attributes & FILE_ATTRIBUTE_ARCHIVE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Archived"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_ARCHIVE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_COMPRESSED)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Compressed"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_COMPRESSED_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_DEVICE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Device"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_DEVICE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }
        // already used directory attribute
        if (_attributes & FILE_ATTRIBUTE_ENCRYPTED)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Encrypted"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_ENCRYPTED_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_HIDDEN)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Hidden"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_HIDDEN_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        /*if (_attributes & FILE_ATTRIBUTE_INTEGRITY_STREAM)
        {
        }
        if (_attributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)
        {
        }
        if (_attributes & FILE_ATTRIBUTE_NO_SCRUB_DATA)
        {
        }*/

        if (_attributes & FILE_ATTRIBUTE_OFFLINE)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Offline"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_OFFLINE_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_READONLY)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Read-only"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_READONLY_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Recalled on access"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_RECALL_ON_OPEN)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Recalled on open"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_RECALL_ON_OPEN_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        /*if (_attributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
        }
        if (_attributes & FILE_ATTRIBUTE_SPARSE_FILE)
        {
        }*/

        if (_attributes & FILE_ATTRIBUTE_SYSTEM)
        {
            _isSystem = true;
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"System"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_SYSTEM_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_TEMPORARY)
        {
            FontIcon f{};
            ToolTipService::SetToolTip(f, box_value(L"Temporary"));
            f.Glyph(unbox_value<hstring>(resources.TryLookup(box_value(L"FILE_ATTRIBUTE_TEMPORARY_IconGlyph"))));
            attributesStackPanel().Children().Append(f);
        }

        if (_attributes & FILE_ATTRIBUTE_VIRTUAL)
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
        bool refresh = true;
        uint_fast64_t newSize = fileSize + args.as<uint_fast64_t>();
        refresh = (newSize / static_cast<double>(fileSize.load())) > 10;

        if (loaded && refresh)
        {
            fileSize.exchange(newSize);
            double displayFileSize = static_cast<double>(newSize);

            DispatcherQueue().TryEnqueue([this, &c_newSize = displayFileSize]()
            {
                hstring c_ext = ::Winnerino::format_size(&c_newSize, 2);
                FileSizeExtensionTextBlock().Text(c_ext);
                FileSizeTextBlock().Text(to_hstring(c_newSize));
                //_displayFileSize = c_newSize;
            });
        }
    }

    inline void FileEntryView::UpdateSize(uint_fast64_t const& size)
    {
        fileSize.exchange(size, std::memory_order_relaxed);
        _displayFileSize = static_cast<double>(size);
        _fileSizeExtension = ::Winnerino::format_size(&_displayFileSize, 2);
    }
}
