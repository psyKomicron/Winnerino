#pragma once

#include "FileEntryView.g.h"
#include <shlwapi.h>
#include "DirectorySizeCalculator.h"

using namespace std;
using namespace winrt;

namespace winrt::Winnerino::implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView>
    {
    public:
        FileEntryView();
        FileEntryView(hstring const& cFileName, hstring const& path, uint64_t fileSize, int64_t attributes);
        FileEntryView(hstring const& systemPath);
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
        bool ShowFilePath()
        {
            return _showFilePath;
        };
        void ShowFilePath(bool value)
        {
            _showFilePath = value;
            e_propertyChanged(*this, PropertyChangedEventArgs{ L"ShowFilePath" });
        };
 
        void Delete();
        Windows::Foundation::IAsyncAction Rename(hstring const& newName, bool const& generateUniqueName = false);

        void MenuFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void FileSizeFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLoaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnUnloaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction ToolTip_Opened(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        bool loaded = true;
        Concurrency::cancellation_token_source cancellationToken{};
        event_token loadedEventToken;
        event_token unloadedEventToken;
        event_token sizeProgressToken;
        ::Winnerino::Storage::DirectorySizeCalculator calculator{};
        double _displayFileSize = 0;
        PERCEIVED perceivedFileType;
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
        bool _showFilePath = false;

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
