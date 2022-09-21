#pragma once

#include "FileEntryView.g.h"
#include <shlwapi.h>
#include <atomic>
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
        event_token Deleted(Windows::Foundation::TypedEventHandler<Winnerino::FileEntryView, Windows::Foundation::IInspectable> const& handler)
        {
            return e_deleted.add(handler);
        };
        void Deleted(event_token const& token)
        {
            e_deleted.remove(token);
        };

        uint64_t FileBytes() const;
        hstring FileName() const;
        hstring FilePath() const;
        hstring FileIcon() const;
        hstring FileSize() const;
        hstring FileExtension() const;
        bool IsDirectory() const;
        bool IsSystem() const;
        hstring OpensWith() const;
        hstring PerceivedType() const;
        Windows::Foundation::DateTime LastWrite() const;
        void LastWrite(Windows::Foundation::DateTime const& value);
        bool IsFileDangerous() const;
        bool ShowFilePath() const;
        void ShowFilePath(bool value);
 
        void Delete();
        Windows::Foundation::IAsyncAction Rename(hstring const& newName, bool const& generateUniqueName = false);

        void MenuFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void FileSizeFlyoutItem_Click(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLoaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnUnloaded(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        Windows::Foundation::IAsyncAction ToolTip_Opened(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void Grid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Grid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void ToolTip_Closed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        atomic_bool loaded = false;
        atomic<uint64_t> fileSize = 1;
        Concurrency::cancellation_token_source cancellationToken{};
        ::Winnerino::Storage::DirectorySizeCalculator calculator{};
        event_token sizeProgressToken;
        event_token loadedEventToken;
        event_token unloadedEventToken;
        double _displayFileSize = 0;
        PERCEIVED perceivedFileType = PERCEIVED_TYPE_UNKNOWN;
        hstring _fileSizeExtension;
        hstring _fileName;
        hstring _filePath;
        hstring _icon;
        hstring _fileExtension;
        uint64_t attributes = 0;
        bool _isDangerous = false;
        bool _isDirectory = false;
        bool _isSystem = false;
        Windows::Foundation::DateTime _lastWrite;
        hstring _opensWith;
        hstring _perceivedType;
        bool _showFilePath = false;
        winrt::Windows::Media::Playback::MediaPlayer player = nullptr;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
        event< Windows::Foundation::TypedEventHandler<Winnerino::FileEntryView, Windows::Foundation::IInspectable> > e_deleted;

        void GetAttributes(int64_t attributes);
        void GetIcon(PCWSTR const& ext);
        void InitFile(PCWSTR ext);
        void ProgressHandler(Windows::Foundation::IInspectable const&, Windows::Foundation::IReference<uint_fast64_t> const& newSize);
        inline void UpdateSize(uint_fast64_t const& size);
        inline bool _IsDangerous();
        inline bool _IsDirectory();
        inline bool _IsSystem();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FileEntryView : FileEntryViewT<FileEntryView, implementation::FileEntryView>
    {
    };
}
