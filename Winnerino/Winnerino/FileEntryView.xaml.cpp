#include "pch.h"
#include "FileEntryView.xaml.h"
#if __has_include("FileEntryView.g.cpp")
#include "FileEntryView.g.cpp"
#endif
#include "shlwapi.h"
#include <math.h>
#include "DirectorySizeCalculator.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Documents;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Foundation;

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
        fileName = cFileName;
        filePath = path;
        if (attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            isDirectory = true;
            IInspectable dirIcon = Application::Current().Resources().TryLookup(box_value(L"DirectoryFontIconGlyph"));
            icon = unbox_value_or(dirIcon, L"\uF142");

            InitializeComponent();
            if (fileName != L"." && fileName != L"..")
            {
                concurrency::create_task([this]()
                {
                    try
                    {
                        DirectorySizeCalculator calc{};
                        event_token token = calc.Progress({ this, &FileEntryView::ProgressHandler });
                        int_fast64_t dirSize = calc.getSize(filePath + L"\\", true);
                        calc.Progress(token);
                        updateSize(dirSize);

                        DispatcherQueue().TryEnqueue([this]()
                        {
                            FileSizeTextBlock().Text(to_hstring(displayFileSize));
                            FileSizeExtensionTextBlock().Text(fileSizeExtension);
                        });
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
                });
            }
        }
        else
        {
            updateSize(fileSize);

            PCWSTR ext = PathFindExtension(fileName.c_str());
            getIcon(ext);

            InitializeComponent(); // we can use UI properties from now on

            FileSizeTextBlock().Text(to_hstring(displayFileSize));
            FileSizeExtensionTextBlock().Text(fileSizeExtension);

            DWORD bufferSize = 0;
            ASSOCSTR assocation = ASSOCSTR::ASSOCSTR_FRIENDLYAPPNAME;
            HRESULT result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", NULL, &bufferSize);
            if (result == S_FALSE)
            {
                //LPTSTR string = (LPTSTR)malloc(sizeof(LPTSTR) * bufferSize);
                LPWSTR string = new WCHAR[bufferSize];
                result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", string, &bufferSize);
                if (result == S_OK)
                {
                    hstring mime = to_hstring(string);
                    openWithFlyoutItem().Text(openWithFlyoutItem().Text() + L" with " + mime);
                }
                delete[] string;
            }
        }

        getAttributes(attributes);
    }

    void FileEntryView::MenuFlyoutItem_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try
        {
            DataPackage dataPackage{};
            dataPackage.SetText(filePath);
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
        OutputDebugString((to_hstring(fileSize) + L"\n").c_str());
    }

    void FileEntryView::ProgressHandler(Windows::Foundation::IInspectable const&, IReference<uint_fast64_t> const& args)
    {
        uint_fast64_t newSize = args.as<uint_fast64_t>();
        fileSize += newSize;
        double _displayFileSize = static_cast<double>(fileSize);
        hstring c_ext = formatSize(&_displayFileSize);
        if (_displayFileSize != displayFileSize)
        {
            DispatcherQueue().TryEnqueue([this, c_newSize = _displayFileSize, c_ext]()
            {
                FileSizeExtensionTextBlock().Text(c_ext);
                FileSizeTextBlock().Text(to_hstring(c_newSize));
            });
        }
        
    }

    void FileEntryView::getAttributes(int64_t attributes)
    {
        ResourceDictionary resources = Application::Current().Resources();
        std::vector<IInspectable> icons{};
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
            isSystem = true;
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

    inline hstring FileEntryView::formatSize(double* size)
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

    inline void FileEntryView::updateSize(uint_fast64_t const& size)
    {
        this->fileSize = size;
        displayFileSize = static_cast<double>(size);
        fileSizeExtension = formatSize(&displayFileSize);
    }

    void FileEntryView::getIcon(PCWSTR const& ext)
    {
        hstring extension = to_hstring(ext);
        if (extension == L".pdf")
        {
            icon = L"\uea90";
        }
        else if (extension == L".gif")
        {
            icon = L"\uf4a9";
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
                icon = key;
            }
            else
            {
                icon = L"\uF142";
            }
        }
    }
}
