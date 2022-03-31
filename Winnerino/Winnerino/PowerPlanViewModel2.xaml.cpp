#include "pch.h"
#include "PowerPlanViewModel2.xaml.h"
#if __has_include("PowerPlanViewModel2.g.cpp")
#include "PowerPlanViewModel2.g.cpp"
#include "powrprof.h"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Windows::Devices;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    PowerPlanViewModel2::PowerPlanViewModel2()
    {
        InitializeComponent();
    }

    PowerPlanViewModel2::PowerPlanViewModel2(const GUID& uuid, const winrt::hstring& name)
    {
        planName = name;
        guid = uuid;

        InitializeComponent();

        // register for notifications
        registerForNotifications();
    }

    PowerPlanViewModel2::~PowerPlanViewModel2()
    {
        if (effectivePowerModeRegistrationHandle)
        {
            PowerUnregisterFromEffectivePowerModeNotifications(effectivePowerModeRegistrationHandle);
        }
        if (powerSettingNotificationRegistrationHandle)
        {
            PowerSettingUnregisterNotification(powerSettingNotificationRegistrationHandle);
        }
    }

    winrt::hstring PowerPlanViewModel2::PowerPlanName()
    {
        return planName;
    }

    winrt::hstring PowerPlanViewModel2::PowerPlanGuid()
    {
        return to_hstring(guid);
    }

    void PowerPlanViewModel2::button_Click(winrt::Windows::Foundation::IInspectable const& sender, RoutedEventArgs const& e)
    {
        /*active = !active;
        powerModeStatusIcon().Glyph(active ? L"\uE73E" : L"");*/
    }

    void onPowerModeChange(EFFECTIVE_POWER_MODE mode, void* context)
    {
        switch (mode)
        {
            case EffectivePowerModeBatterySaver:
                OutputDebugString(L"Battery saver on.\n");
                break;
            case EffectivePowerModeBetterBattery:
                OutputDebugString(L"Mode change: Better battery.\n");
                break;
            case EffectivePowerModeBalanced:
                OutputDebugString(L"Mode change: Balanced.\n");
                break;
            case EffectivePowerModeHighPerformance:
                OutputDebugString(L"Mode change: High performance.\n");
                break;
            case EffectivePowerModeMaxPerformance:
                OutputDebugString(L"Mode change: Max performance.\n");
                break;
            case EffectivePowerModeGameMode:
                OutputDebugString(L"Mode change: Game mode enabled.\n");
                break;
            case EffectivePowerModeMixedReality:
                OutputDebugString(L"Mode change: Mixed reality enabled.\n");
                break;
            default:
                OutputDebugString(L"Power mode changed, but is not recognized.\n");
                break;

        }
    }

    void callback() {}

    void PowerPlanViewModel2::registerForNotifications() noexcept
    {
        HRESULT result = PowerRegisterForEffectivePowerModeNotifications(EFFECTIVE_POWER_MODE_V1, onPowerModeChange, (void *)this, &effectivePowerModeRegistrationHandle);
        if (SUCCEEDED(result))
        {
            OutputDebugString(L"Registered for power mode changes.\n");
        }
        else
        {
            effectivePowerModeRegistrationHandle = NULL;
        }
        HPOWERNOTIFY tempHandle = NULL;
        if (PowerSettingRegisterNotification(&GUID_POWERSCHEME_PERSONALITY, DEVICE_NOTIFY_CALLBACK, &callback, &tempHandle))
        {
            OutputDebugString(L"Registered for power setting notifications.");
        }
        else
        {
            hstring message = to_hstring(std::system_category().message(GetLastError()));
            OutputDebugString(message.c_str());
        }
        //PowerSettingUnregisterNotification()
    }
}
