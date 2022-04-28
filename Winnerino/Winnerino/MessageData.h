#pragma once

namespace winrt::Winnerino
{
    class MessageData
    {
    private:
        winrt::hstring message;
        winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity;

    public:
        MessageData(winrt::hstring const& message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity const& severity);
        winrt::hstring GetDataMessage();
        winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity GetSeverity();
    };
}

