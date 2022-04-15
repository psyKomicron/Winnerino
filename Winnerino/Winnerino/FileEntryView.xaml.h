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
        double displayFileSize = 0;
        hstring fileSizeExtension;
        hstring fileName;
        hstring filePath;
        int64_t fileSize = 0;
        hstring icon;
        bool isDirectory = false;
        bool isSystem = false;

        void getAttributes(int64_t attributes);
        void getIcon(PCWSTR ext);
        hstring formatSize(double* size);

    public:
        /// <summary>
        /// Default constructor
        /// </summary>
        FileEntryView();
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="cFileName">Name</param>
        /// <param name="path">Path</param>
        /// <param name="fileSize">Size in bytes</param>
        /// <param name="attributes">Attributes (enumeration)</param>
        FileEntryView(hstring const& cFileName, hstring const& path, int64_t fileSize, int64_t attributes);

        /// <summary>
        /// Name.
        /// </summary>
        /// <returns>Name of this file</returns>
        hstring FileName() { return fileName; };
        /// <summary>
        /// Path.
        /// </summary>
        /// <returns>Path of this file</returns>
        hstring FilePath() { return filePath; };
        /// <summary>
        /// Size of the file, formatted. Use with FileEntryView::FileSizeExtension()
        /// </summary>
        /// <returns>Size of the file, formatted.</returns>
        hstring FileSize() { return to_hstring(displayFileSize); };
        /// <summary>
        /// Extension for the formatted size. (Kb, Mb...)
        /// </summary>
        /// <returns>Extension for the formatted size</returns>
        hstring FileSizeExtension() { return fileSizeExtension; };
        /// <summary>
        /// Unicode icon (Segoe Fluent Icons) for the file.
        /// </summary>
        /// <returns>Unicode icon</returns>
        hstring FileIcon() { return icon; };
        /// <summary>
        /// Tells if this file is a directory.
        /// </summary>
        /// <returns>True if is directory, false otherwise</returns>
        bool IsDirectory() { return isDirectory; };
        /// <summary>
        /// Tells if this file is managed by the system.
        /// </summary>
        /// <returns>True if the file is managed by the system</returns>
        bool IsSystem() { return isSystem; };
        void MenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView, implementation::FileEntryView>
    {
    };
}
