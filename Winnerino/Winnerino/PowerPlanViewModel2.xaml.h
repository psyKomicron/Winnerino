#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "PowerPlanViewModel2.g.h"
#include "powersetting.h"

namespace winrt::Winnerino::implementation
{
    struct PowerPlanViewModel2 : PowerPlanViewModel2T<PowerPlanViewModel2>
    {
    private:
        bool active = false;
        EFFECTIVE_POWER_MODE powerMode;
        GUID guid = GUID();
        winrt::hstring planName;
        void* effectivePowerModeRegistrationHandle = NULL;
        HPOWERNOTIFY powerSettingNotificationRegistrationHandle = NULL;

        void registerForNotifications() noexcept;

    public:
        PowerPlanViewModel2();
        PowerPlanViewModel2(const GUID& uuid, const winrt::hstring& name);
        ~PowerPlanViewModel2();

        winrt::hstring PowerPlanName();
        winrt::hstring PowerPlanGuid();
        void button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct PowerPlanViewModel2 : PowerPlanViewModel2T<PowerPlanViewModel2, implementation::PowerPlanViewModel2>
    {
    };
}
