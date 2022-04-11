#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "FileEntryView.g.h"

using namespace winrt;

namespace winrt::Winnerino::implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView>
    {
    private:
        hstring fileName;
        int64_t fileSize = 0;
        double displayFileSize = 0;
        hstring fileSizeExtension;
        int isDirectory = -1;
        bool isSystem = false;

        void getAttributes(int64_t attributes);
        hstring formatSize(double* size);

    public:
        FileEntryView();
        FileEntryView(hstring const& cFileName, hstring const& extension, int64_t fileSize, int64_t attributes);

        hstring FileName() { return fileName; };
        hstring FileSize() { return to_hstring(displayFileSize); };
        hstring FileSizeExtension() { return fileSizeExtension; };
        hstring FileIcon();
        bool IsDirectory() { return isDirectory == 2; };
        bool IsSystem() { return isSystem; };
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView, implementation::FileEntryView>
    {
    };
}
