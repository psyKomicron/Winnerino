#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
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
            return m_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            m_propertyChanged.remove(token);
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
 
        int16_t Compare(Winnerino::FileEntryView const& other);

        void MenuFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void FileSizeFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLoaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnUnloaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction OpenInExplorerFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction ToolTip_Opened(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OpenWithFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void DeleteFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction RenameFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction CopyFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction CutFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        bool loaded = true;
        event_token loadedEventToken;
        event_token unloadedEventToken;
        Concurrency::task<void> calculateSizeTask;
        Concurrency::cancellation_token_source cancellationToken{};
        //PERCEIVED perceivedFileType;
        double _displayFileSize = 0;
        hstring _fileSizeExtension;
        hstring _fileName;
        hstring _filePath;
        uint64_t fileSize = 0;
        hstring _icon;
        bool _isDirectory = false;
        bool _isSystem = false;
        Windows::Foundation::DateTime _lastWrite;
        hstring _opensWith;
        hstring _perceivedType;
        bool _isDangerous = false;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

        inline hstring FormatSize(double* size);
        void GetAttributes(int64_t attributes);
        void GetIcon(PCWSTR const& ext);
        void InitFile(PCWSTR ext);
        void ProgressHandler(Windows::Foundation::IInspectable const&, Windows::Foundation::IReference<uint_fast64_t> const& newSize);
        Windows::Foundation::IAsyncAction RenameFile(hstring newName, const bool& generateUnique = false);
        inline void UpdateSize(uint_fast64_t const& size);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView, implementation::FileEntryView>
    {
    };
}
