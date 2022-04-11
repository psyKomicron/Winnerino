#include "pch.h"
#include "FileEntryView.xaml.h"
#if __has_include("FileEntryView.g.cpp")
#include "FileEntryView.g.cpp"
#include "shlwapi.h"
#include <math.h>
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Documents;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    FileEntryView::FileEntryView()
    {
        InitializeComponent();
    }

    FileEntryView::FileEntryView(hstring const& cFileName, hstring const&, int64_t fileSize, int64_t attributes)
    {
        fileName = cFileName;
        this->fileSize = fileSize;
        displayFileSize = static_cast<double>(fileSize);
        fileSizeExtension = formatSize(&displayFileSize);
        // get attributes
        isDirectory = attributes & FILE_ATTRIBUTE_DIRECTORY ? 2 : 1;
        InitializeComponent();

        if (isDirectory == 1)
        {
            PCWSTR ext = PathFindExtension(cFileName.c_str());
            if (ext != NULL)
            {
                PWSTR perceived = NULL;
                PERCEIVED perceivedType;
                PERCEIVEDFLAG perceivedFlag;
                HRESULT result = AssocGetPerceivedType(ext, &perceivedType, &perceivedFlag, &perceived);
                if (result == S_OK)
                {
                    fileTypeTextBlock().Text(to_hstring(perceived) + L" | " + to_hstring(perceivedType));
                }
                else
                {
                    MainWindow::Current().notifyError(result);
                }
            }
            else
            {
                MainWindow::Current().notifyUser(L"Extension not found", InfoBarSeverity::Warning);
            }
        }

        getAttributes(attributes);
    }

    hstring FileEntryView::FileIcon()
    {
        if (isDirectory == 2)
        {
            IInspectable dirIcon = Application::Current().Resources().TryLookup(box_value(L"DirectoryFontIconGlyph"));
            return unbox_value_or(dirIcon, L"\uF142");
        }
#if FALSE
        else if (isDirectory == 1)
        {
            IInspectable fileIcon = Application::Current().Resources().TryLookup(box_value(L"FileFontIconGlyph"));
            return unbox_value_or(fileIcon, L"\uF142");
        }
#endif // FALSE

        else
        {
            return L"\uF142"; 
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
}
