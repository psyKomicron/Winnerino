#include "pch.h"
#include "PowerModePage.xaml.h"
#if __has_include("PowerModePage.g.cpp")
#include "PowerModePage.g.cpp"
#endif
#include "guiddef.h"
#include "powrprof.h"

using namespace std;
using namespace winrt;

using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Data;

using namespace Windows::Foundation;

using namespace Winnerino;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    PowerModePage::PowerModePage()
    {
        InitializeComponent();
    }

    bool PowerModePage::EditPowerPlansButtonEnabled()
    {
        return viewModels.size() > 0;
    }

    winrt::event_token PowerModePage::PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChangedEvent.add(handler);
    }

    void PowerModePage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChangedEvent.remove(token);
    }

    void PowerModePage::listPowerPlans()
    {
        ULONG index = 0;
        GUID guid{};
        DWORD bufferSize{};

        if (PowerEnumerate(NULL, NULL, NULL, ACCESS_SCHEME, index, NULL, &bufferSize) == ERROR_MORE_DATA)
        {
            while (PowerEnumerate(NULL, NULL, NULL, ACCESS_SCHEME, index, (UCHAR*)&guid, &bufferSize) == ERROR_SUCCESS)
            {
                DWORD friendlyNameBufferSize{};
                if (PowerReadFriendlyName(NULL, &guid, NULL, NULL, NULL, &friendlyNameBufferSize) == ERROR_SUCCESS)
                {
                    PUCHAR nameBuffer = (PUCHAR)malloc(sizeof(UCHAR) * friendlyNameBufferSize);
                    if (PowerReadFriendlyName(NULL, &guid, NULL, NULL, nameBuffer, &friendlyNameBufferSize) == ERROR_SUCCESS)
                    {
                        hstring powerPlanName = to_hstring((wchar_t*)nameBuffer);
                        PowerPlanViewModel2 viewModel{ guid, powerPlanName };
                        viewModels.push_back(viewModel);

                        powerPlansGrid().Children().Append(viewModel);

                        MenuFlyoutItem item{};
                        item.Text(powerPlanName);
                        item.Tag(box_value(powerPlanName));
                        item.Click({ get_weak(), &PowerModePage::powerPlanItem_Click });
                        editMenuItem().Items().Append(item);
                    }
                    else
                    {
                        std::string message = system_category().message(GetLastError());
                        OutputDebugString(to_hstring(message).c_str());
                    }

                    // always free after malloc
                    free(nameBuffer);
                    index++;
                }
                else
                {
                    OutputDebugString(L"Failed to enumerate power plans.");
                    std::string message = system_category().message(GetLastError());
                    MainWindow::Current().notifyUser(to_hstring(message), InfoBarSeverity::Error);
                }
            }
        }
    }

    void PowerModePage::powerPlanItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        hstring tag = unbox_value<hstring>(unbox_value<MenuFlyoutItem>(sender).Tag());
        for (size_t i = 0; i < viewModels.size(); i++)
        {
            if (viewModels[i].PowerPlanName() == tag)
            {
                MainWindow::Current().notifyUser(L"You choose to edit " + tag + L".", Controls::InfoBarSeverity::Success);
                return;
            }
        }
        MainWindow::Current().notifyUser(L"Could not edit " + tag + L" power plan .", Controls::InfoBarSeverity::Warning);
    }

    void PowerModePage::refreshMenuFlyoutItem_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        // TODO: resync power plans with the system
    }

    void PowerModePage::listPlansMenuFlyoutItem_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        listPowerPlans();
    }

    void PowerModePage::aboutMenuFlyoutItem_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
    }

    void PowerModePage::openWindowsSettingsMenuFlyoutItem_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
    }

    void PowerModePage::Page_Loaded(IInspectable const& sender, RoutedEventArgs const& e)
    {
        listPowerPlans();
        propertyChangedEvent(*this, PropertyChangedEventArgs{ L"EditPowerPlansButtonEnabled" });
    }
}
