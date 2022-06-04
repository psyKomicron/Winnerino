#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "shlwapi.h"
#include "FileEntryView.g.h"

using namespace std;
using namespace winrt;

namespace winrt::Winnerino::implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView>
    {
    public:
        FileEntryView();
        FileEntryView(hstring const& cFileName, hstring const& path, uint64_t fileSize, int64_t attributes);
        ~FileEntryView();

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        uint64_t FileBytes() { return fileSize; };
        hstring FileName() { return _fileName; };
        hstring FilePath() { return _filePath; };
        hstring FileIcon() { return _icon; };
        hstring FileSize() { return to_hstring(_displayFileSize) + _fileSizeExtension; };
        bool IsDirectory() { return _isDirectory; };
        bool IsSystem() { return _isSystem; };
        hstring OpensWith() { return _opensWith; };
        hstring PerceivedType() { return _perceivedType; };
        Windows::Foundation::DateTime LastWrite() { return _lastWrite; };
        void LastWrite(Windows::Foundation::DateTime const& lastWrite) { _lastWrite = lastWrite; };
        bool IsFileDangerous() { return _isDangerous; };
 
        void Delete();
        Windows::Foundation::IAsyncAction Rename(hstring const& newName, bool const& generateUniqueName = false);

        void MenuFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void FileSizeFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLoaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnUnloaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction ToolTip_Opened(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        bool loaded = true;
        Concurrency::task<void> calculateSizeTask;
        Concurrency::cancellation_token_source cancellationToken{};
        double _displayFileSize = 0;
        event_token loadedEventToken;
        PERCEIVED perceivedFileType;
        event_token unloadedEventToken;
        hstring _fileSizeExtension;
        hstring _fileName;
        hstring _filePath;
        uint64_t fileSize = 0;
        hstring _icon;
        bool _isDangerous = false;
        bool _isDirectory = false;
        bool _isSystem = false;
        Windows::Foundation::DateTime _lastWrite;
        hstring _opensWith;
        hstring _perceivedType;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        inline hstring FormatSize(double* size);
        void GetAttributes(int64_t attributes);
        void GetIcon(PCWSTR const& ext);
        void InitFile(PCWSTR ext);
        void ProgressHandler(Windows::Foundation::IInspectable const&, Windows::Foundation::IReference<uint_fast64_t> const& newSize);
        inline void UpdateSize(uint_fast64_t const& size);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView, implementation::FileEntryView>
    {
    };
}
