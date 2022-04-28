#include "pch.h"
#include "MessageData.h"

namespace winrt::Winnerino
{
    MessageData::MessageData(winrt::hstring const& message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity)
    {
        this->message = message;
        this->severity = severity;
    }

    winrt::hstring MessageData::GetDataMessage()
    {
        return message;
    }

    winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity MessageData::GetSeverity()
    {
        return severity;
    }
}