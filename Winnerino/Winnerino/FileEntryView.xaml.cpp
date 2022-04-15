#include "pch.h"
#include "FileEntryView.xaml.h"
#if __has_include("FileEntryView.g.cpp")
#include "FileEntryView.g.cpp"
#endif
#include "shlwapi.h"
#include <math.h>
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

    FileEntryView::FileEntryView(hstring const& cFileName, hstring const& path, int64_t fileSize, int64_t attributes)
    {
        fileName = cFileName;
        this->fileSize = fileSize;
        filePath = path;
        displayFileSize = static_cast<double>(fileSize);
        fileSizeExtension = formatSize(&displayFileSize);
        isDirectory = attributes & FILE_ATTRIBUTE_DIRECTORY;

        PCWSTR ext = PathFindExtension(fileName.c_str());
        getIcon(ext);

        InitializeComponent(); // we can use UI properties from now on

        /*if (result == S_FALSE)
        {
            LPTSTR string = (LPTSTR)malloc(sizeof(LPTSTR) * bufferSize);
            if (string != NULL)
            {
                result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", string, &bufferSize);
                if (result == S_OK)
                {
                    try
                    {
                        BitmapImage image{ Uri{ to_hstring(string) }};
                        iconImage().Source(image);
                    }
                    catch (const std::exception& e)
                    {
                        OutputDebugString(to_hstring(e.what()).c_str());
                    }
                }
                free(string);
            }
        }*/
        getAttributes(attributes);
        DWORD bufferSize = 0;
        ASSOCSTR assocation = ASSOCSTR::ASSOCSTR_FRIENDLYAPPNAME;
        HRESULT result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", NULL, &bufferSize);
        if (result == S_FALSE)
        {
            LPTSTR string = (LPTSTR)malloc(sizeof(LPTSTR) * bufferSize);
            if (string != NULL)
            {
                result = AssocQueryString(ASSOCF_INIT_FIXED_PROGID | ASSOCF_INIT_FOR_FILE, assocation, ext, L"open", string, &bufferSize);
                if (result == S_OK)
                {
                    hstring mime = to_hstring(string);
                    openWithFlyoutItem().Text(openWithFlyoutItem().Text() + L" with " + mime);
                }
                free(string);
            }
        }
        
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


    void FileEntryView::getAttributes(int64_t attributes)
    {
        ResourceDictionary resources = Application::Current().Resources();
        std::vector<IInspectable> icons{};
        if (attributes & FILE_ATTRIBUTE_NORMAL)
        {
            // no attributes
        }
        else
        {
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
    }

    hstring FileEntryView::formatSize(double* size)
    {
        if (*size >= 0x10000000000)
        {
            *size = round(*size / 0x10000000000);
            return L" Tb";
        }
        if (*size >= 0x40000000)
        {
            *size *= (double)1000;
            *size = round(*size / 0x40000000);
            *size /= (double)1000;
            return L" Gb";
        }
        if (*size >= 0x100000)
        {
            *size = round(*size / 0x100000);
            return L" Mb";
        }
        if (*size >= 0x400)
        {
            *size = round(*size / 0x400);
            return L" Kb";
        }
        return L" b";
    }

    void FileEntryView::getIcon(PCWSTR ext)
    {
        if (isDirectory)
        {
            IInspectable dirIcon = Application::Current().Resources().TryLookup(box_value(L"DirectoryFontIconGlyph"));
            icon = unbox_value_or(dirIcon, L"\uF142");
        }
        else
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
}
