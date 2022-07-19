#include "pch.h"
#include "SpotlightImporter.xaml.h"
#if __has_include("SpotlightImporter.g.cpp")
#include "SpotlightImporter.g.cpp"
#endif

#include <shlwapi.h>
#include <vector>
#include <shobjidl.h>
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

    void SpotlightImporter::UserControl_Loading(FrameworkElement const&, IInspectable const& args)
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

    void SpotlightImporter::ImportButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (chosenFolder)
        {
            CopyProgressBar().Value(0);

            DirectoryEnumerator enumerator{};
            hstring spotlightPath = UserDataPaths::GetDefault().LocalAppData() + L"\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets";
            vector<FileInfo>* files = enumerator.GetFiles(spotlightPath);
            if (files && files->size() > 0)
            {
                CopyProgressBar().Maximum(files->size());
                
                for (size_t i = 0; i < files->size(); i++)
                {
                    WCHAR combined[ALTERNATE_MAX_PATH](0);
                    PathCombine(combined, chosenFolder.Path().c_str(), files->at(i).Name().c_str());
                    hstring newPath = to_hstring(combined) + L".png";
                    if (!CopyFile(files->at(i).Path().c_str(), newPath.c_str(), false))
                    {
                        MainWindow::Current().NotifyUser(L"Failed to copy " + files->at(i).Name(), winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
                    }
                    CopyProgressBar().Value(i + 1);
                }
            }
        }
    }
}