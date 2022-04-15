#include "pch.h"
#include <chrono>
#include "PowerControllerView.xaml.h"
#if __has_include("PowerControllerView.g.cpp")
#include "PowerControllerView.g.cpp"
#endif

using namespace std::chrono_literals;
using namespace std::chrono;

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Data;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Dispatching;

using namespace Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    PowerControllerView::PowerControllerView()
    {
        InitializeComponent();

        dispatcherTimer = DispatcherQueue().CreateTimer();
        dispatcherTimer.Interval(duration(1s));
        tickEventToken = dispatcherTimer.Tick({ get_weak(), &PowerControllerView::dispatcherQueueTime_Tick });
    }

    PowerControllerView::~PowerControllerView()
    {
        if (tickEventToken && dispatcherTimer)
        {
            dispatcherTimer.Tick(tickEventToken);
        }
    }

#pragma region Properties
    bool PowerControllerView::ButtonsEnabled()
    {
        return _buttonsEnabled;
    }

    void PowerControllerView::ButtonsEnabled(bool value)
    {
        _buttonsEnabled = value;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"ButtonsEnabled" });
    }

    bool PowerControllerView::IsReadOnly()
    {
        return _isReadOnly;
    }

    void PowerControllerView::IsReadOnly(bool value)
    {
        _isReadOnly = value;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"IsReadOnly" });
    }

    int32_t PowerControllerView::Hours()
    {
        return _hours;
    }

    void PowerControllerView::Hours(int32_t value)
    {
        if (value < 0)
        {
            return;
        }

        _hours = value;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Hours" });
    }

    int32_t PowerControllerView::Minutes()
    {
        return _minutes;
    }

    void PowerControllerView::Minutes(int32_t value)
    {
        if (value < 0)
        {
            return;
        }

        _minutes = value;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Minutes" });
    }

    int32_t PowerControllerView::Seconds()
    {
        return _seconds;
    }

    void PowerControllerView::Seconds(int32_t value)
    {
        if (value < 0)
        {
            return;
        }

        _seconds = value;
        m_propertyChanged(*this, PropertyChangedEventArgs{ L"Seconds" });
    }
#pragma endregion

    event_token PowerControllerView::PropertyChanged(PropertyChangedEventHandler const& value)
    {
        return m_propertyChanged.add(value);
    }

    void PowerControllerView::PropertyChanged(event_token const& token)
    {
        m_propertyChanged.remove(token);
    }

    void PowerControllerView::executeController()
    {
        auto index = actionComboBox().SelectedIndex();

        switch (index)
        {
            case 0: // Lock
                controller.lock();
                break;
            case 1: // Sleep
                controller.sleep();
                break;
            case 2: // Restart
                // not yet done.
                break;
            case 3: // Shutdown
                controller.shutdown();
                break;
            case 4: // Hibenate
                controller.sleep(true, false);
                break;
            default:
                OutputDebugString(L"Controller action not recognized.");
                break;
        }
    }

    void PowerControllerView::restartTimer()
    {
        restartTimerButton().IsEnabled(false);
        startTimerButton().IsEnabled(false);

        dispatcherTimer.Stop();
        currentTimeSpan = TimeSpan{ originalTimeSpan };
        updateView(currentTimeSpan);
        if (isTimerRunning)
        {
            dispatcherTimer.Start();
        }

        restartTimerButton().IsEnabled(true);
        startTimerButton().IsEnabled(true);
    }

    void PowerControllerView::startTimer()
    {
        originalTimeSpan = hours(Hours());
        originalTimeSpan += minutes(Minutes());
        originalTimeSpan += seconds(Seconds());
        currentTimeSpan = TimeSpan{ originalTimeSpan };

        dispatcherTimer.Start();
        isTimerRunning = true;

        startTimerButtonIcon().Symbol(Symbol::Pause);        
        ButtonsEnabled(false);
        IsReadOnly(true);
    }

    void PowerControllerView::stopTimer()
    {
        dispatcherTimer.Stop();
        isTimerRunning = false;

        ButtonsEnabled(true);
        IsReadOnly(false);
        startTimerButtonIcon().Symbol(Symbol::Play);
    }

    void PowerControllerView::updateView(winrt::Windows::Foundation::TimeSpan timeSpan)
    {   
        hours hours = duration_cast<std::chrono::hours>(timeSpan);
        timeSpan -= hours;
        minutes minutes = duration_cast<std::chrono::minutes>(timeSpan);
        timeSpan -= minutes;
        seconds seconds = duration_cast<std::chrono::seconds>(timeSpan);

        // Update text boxes
        Hours(hours.count());
        Minutes(minutes.count());
        Seconds(seconds.count());
    }

#pragma region EventHandlers
    void PowerControllerView::dispatcherQueueTime_Tick(const IInspectable&, const IInspectable&)
    {
        currentTimeSpan -= seconds(1);
        updateView(currentTimeSpan);

        if ((currentTimeSpan / seconds(1)) == 0)
        {
            stopTimer();
            currentTimeSpan = TimeSpan{ originalTimeSpan };
            updateView(currentTimeSpan);
            executeController();
        }
        else if ((currentTimeSpan / seconds(30)) == 0)
        {
            hstring action;
            int index = actionComboBox().SelectedIndex();
            switch (index)
            {
                case 0: // Lock
                    action = L"Computer will lock";
                    break;
                case 1: // Sleep
                    action = L"Computer will sleep";
                    break;
                case 2: // Restart
                    // not yet done.
                    break;
                case 3: // Shutdown
                    action = L"Computer will shutdown";
                    break;
                case 4: // Hibenate
                    action = L"Computer will hibernate";
                    break;
                default:
                    OutputDebugString(L"Controller action not recognized.");
                    break;
            }
            MainWindow::Current().NotifyUser(action + L" in 30 seconds.", InfoBarSeverity::Informational);
        }
    }

    void PowerControllerView::hoursUpButton_Click(IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        Hours(Hours() + 1);
    }

    void PowerControllerView::hoursDownButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Hours(Hours() - 1);
    }

    void PowerControllerView::minutesUpButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Minutes(Minutes() + 1);
    }

    void PowerControllerView::minutesDownButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Minutes(Minutes() - 1);
    }

    void PowerControllerView::secondsUpButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Seconds(Seconds() + 1);
    }

    void PowerControllerView::secondsDownButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        Seconds(Seconds() - 1);
    }

    void PowerControllerView::textBox_GotFocus(IInspectable const&, RoutedEventArgs const&)
    {
        /*TextBox box = sender.try_as<TextBox>();
        if (box.Text() == L"0")
        {
            hoursTextBox().Text();
        }*/
    }

    void PowerControllerView::restartTimerButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        restartTimer();
    }

    void PowerControllerView::startTimerButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (isTimerRunning)
        {
            stopTimer();
        }
        else
        {
            startTimer();
        }
    }

    void PowerControllerView::textBox_BeforeTextChanging(TextBox const&, TextBoxBeforeTextChangingEventArgs const& args)
    {
        args.Cancel(!(args.NewText().size() > 0 && args.NewText()[0] != '-'));
    }
#pragma endregion
}
