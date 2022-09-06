#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "SpotlightImporter.g.h"

namespace winrt::Winnerino::implementation
{
    struct SpotlightImporter : SpotlightImporterT<SpotlightImporter>
    {
    public:
        SpotlightImporter();

        bool ButtonsEnabled();

        Windows::Foundation::IAsyncAction UserControl_Loading(winrt::Microsoft::UI::Xaml::FrameworkElement const& sender, winrt::Windows::Foundation::IInspectable const& args);
        Windows::Foundation::IAsyncAction ChooseFolderButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        Windows::Foundation::IAsyncAction ImportButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

    private:
        Windows::Storage::StorageFolder chosenFolder = nullptr;
        bool _buttonsEnabled = true;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct SpotlightImporter : SpotlightImporterT<SpotlightImporter, implementation::SpotlightImporter>
    {
    };
}
