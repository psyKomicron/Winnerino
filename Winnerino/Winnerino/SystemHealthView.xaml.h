#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "SystemHealthView.g.h"

namespace winrt::Winnerino::implementation
{
    struct SystemHealthView : SystemHealthViewT<SystemHealthView>
    {
    public:
        SystemHealthView();

        event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value)
        {
            return e_propertyChanged.add(value);
        };
        void PropertyChanged(event_token const& token)
        {
            e_propertyChanged.remove(token);
        };

        double SystemGeneralHealth() { return _systemGeneralHealth; };
        void SystemGeneralHealth(double const& value)
        {
            _systemGeneralHealth = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"SystemGeneralHealth" });
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"SystemGeneralForeground" });
        };
        double TotalUsedSpace() { return _totalUsedSpace; };
        void TotalUsedSpace(double const& value)
        {
            _totalUsedSpace = value;
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"TotalUsedSpace" });
            e_propertyChanged(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ L"TotalUsedForeground" });
        };
        winrt::Microsoft::UI::Xaml::Media::Brush TotalUsedForeground();
        winrt::Microsoft::UI::Xaml::Media::Brush SystemGeneralForeground();

        void DeviceNameBox_QuerySubmitted(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxQuerySubmittedEventArgs const& args);

    private:
        double _systemGeneralHealth = 0;
        double _totalUsedSpace = 0;

        event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> e_propertyChanged;

        void GetSmartInfos(winrt::hstring const& deviceName);
        winrt::Windows::Foundation::IAsyncAction LoadDrivesInfos();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct SystemHealthView : SystemHealthViewT<SystemHealthView, implementation::SystemHealthView>
    {
    };
}
