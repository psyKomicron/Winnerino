#include "pch.h"
#include "SpotlightImporter.xaml.h"
#if __has_include("SpotlightImporter.g.cpp")
#include "SpotlightImporter.g.cpp"
#endif

#include <shlwapi.h>
#include <vector>
#include <shobjidl_core.h>
#include <string>
#include "winrt/Windows.Storage.Pickers.h"
#include "DirectoryEnumerator.h"

using namespace std;
using namespace ::Winnerino::Storage;
using namespace winrt;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    SpotlightImporter::SpotlightImporter()
    {
        InitializeComponent();
    }

    bool SpotlightImporter::ButtonsEnabled()
    {
        return _buttonsEnabled;
    }

    IAsyncAction SpotlightImporter::UserControl_Loading(FrameworkElement const&, IInspectable const& args)
    {
        // TODO: get string from settings/resources/online
        hstring spotlightPath = UserDataPaths::GetDefault().LocalAppData() + L"\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets";
        SpotlightPathTextBlock().Text(hstring(spotlightPath));
        if (PathFileExists(spotlightPath.c_str()))
        {
            DirectoryEnumerator enumerator{};
            unique_ptr<vector<FileInfo>> files(enumerator.GetFiles(spotlightPath));
            if (files->size() == 0)
            {
                _buttonsEnabled = false;
                e_propertyChanged(*this, Data::PropertyChangedEventArgs(L"ButtonsEnabled"));
            }
        }

        auto&& dirs = co_await KnownFolders::PicturesLibrary().GetFoldersAsync();
        if (dirs.Size() > 0)
        {
            hstring p = (co_await dirs.GetAt(0).GetParentAsync()).Path();

            DispatcherQueue().TryEnqueue([this, cname = p + L"\\Spotlight"]()
            {
                ChosenFolderPath().Text(cname);
            });
        }
    }

    IAsyncAction SpotlightImporter::ChooseFolderButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        //SpotlightImporter lifetime = get_strong();

        FolderPicker picker{};
        // HACK: https://github.com/microsoft/WindowsAppSDK/issues/1063
        HWND mainWindowHandle = GetWindowFromWindowId(MainWindow::Current().Id());
        picker.as<IInitializeWithWindow>()->Initialize(mainWindowHandle);
        picker.FileTypeFilter().ReplaceAll({ L"*" });
        picker.SettingsIdentifier(L"Spotlight Importer");
        picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
        picker.ViewMode(PickerViewMode::List);

        chosenFolder = co_await picker.PickSingleFolderAsync();
        if (chosenFolder)
        {
            DispatcherQueue().TryEnqueue([this]()
            {
                ChosenFolderPath().Text(chosenFolder.Path());
            });
        }
    }

    IAsyncAction SpotlightImporter::ImportButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!chosenFolder)
        {
            chosenFolder = co_await KnownFolders::PicturesLibrary().CreateFolderAsync(L"Spotlight", CreationCollisionOption::OpenIfExists);
        }

        CopyProgressBar().Value(0);

        hstring spotlightPath = UserDataPaths::GetDefault().LocalAppData() + L"\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets";
        StorageFolder spotlightFolder = co_await StorageFolder::GetFolderFromPathAsync(spotlightPath);
        auto&& files = co_await spotlightFolder.GetFilesAsync();
        if (files.Size() > 0)
        {
            CopyProgressBar().Maximum(files.Size());

            double format = atof(to_string(FormatComboBox().SelectedItem().as<FrameworkElement>().Tag().as<hstring>()).c_str());

            for (size_t i = 0; i < files.Size(); i++)
            {
                auto&& imageProperties = co_await files.GetAt(i).Properties().GetImagePropertiesAsync();
                if (imageProperties.Height() / static_cast<double>(imageProperties.Width()) == format)
                {
                    WCHAR combined[ALTERNATE_MAX_PATH](0);
                    PathCombine(combined, chosenFolder.Path().c_str(), files.GetAt(i).Name().c_str());
                    hstring newPath = to_hstring(combined) + L".png";

                    try
                    {
                        StorageFile newFile = co_await chosenFolder.CreateFileAsync(newPath);
                        co_await files.GetAt(i).CopyAndReplaceAsync(newFile);
                    }
                    catch (hresult_error const& err)
                    {
                        //I18N: Translate.
                        MainWindow::Current().NotifyUser(L"Failed to copy " + files.GetAt(i).Name(), winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
                    } 
                }

                DispatcherQueue().TryEnqueue([this, value = i + 1]()
                {
                    CopyProgressBar().Value(value);
                });
            }
        }

        co_return;
    }
}