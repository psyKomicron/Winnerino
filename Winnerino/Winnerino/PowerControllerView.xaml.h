#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "winrt/Microsoft.UI.Dispatching.h"
#include "PowerControllerView.g.h"
#include "PowerController.h"

namespace winrt::Winnerino::implementation
{
    struct PowerControllerView : PowerControllerViewT<PowerControllerView>
    {
    private:
        // properties
        bool _buttonsEnabled = true;
        bool _isReadOnly = false;
        int32_t _hours = 0;
        int32_t _minutes = 0;
        int32_t _seconds = 0;
        Winnerino::Controllers::PowerController controller = Winnerino::Controllers::PowerController(true);
        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::event_token tickEventToken;
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer dispatcherTimer = nullptr;
        winrt::Windows::Foundation::TimeSpan originalTimeSpan;
        winrt::Windows::Foundation::TimeSpan currentTimeSpan;
        bool isTimerRunning = false;

        void executeController();
        void restartTimer();
        void startTimer();
        void stopTimer();
        void updateView(winrt::Windows::Foundation::TimeSpan timeSpan);
        void dispatcherQueueTime_Tick(const IInspectable&, const IInspectable&);

    public:
        PowerControllerView();
        ~PowerControllerView();

        bool ButtonsEnabled();
        void ButtonsEnabled(bool value);

        bool IsReadOnly();
        void IsReadOnly(bool value);

        int32_t Hours();
        void Hours(int32_t value);

        int32_t Minutes();
        void Minutes(int32_t value);

        int32_t Seconds();
        void Seconds(int32_t value);

        winrt::event_token PropertyChanged(Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& value);
        void PropertyChanged(winrt::event_token const& token);

        void hoursUpButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void hoursDownButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void minutesUpButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void minutesDownButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void secondsUpButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void secondsDownButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void textBox_GotFocus(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void restartTimerButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void startTimerButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void textBox_BeforeTextChanging(winrt::Microsoft::UI::Xaml::Controls::TextBox const& sender, winrt::Microsoft::UI::Xaml::Controls::TextBoxBeforeTextChangingEventArgs const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct PowerControllerView : PowerControllerViewT<PowerControllerView, implementation::PowerControllerView>
    {
    };
}
