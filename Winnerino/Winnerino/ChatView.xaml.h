#pragma once

#include "winrt/Microsoft.UI.Xaml.h"
#include "winrt/Microsoft.UI.Xaml.Markup.h"
#include "winrt/Microsoft.UI.Xaml.Controls.Primitives.h"
#include "ChatView.g.h"

namespace winrt::Winnerino::implementation
{
    struct ChatView : ChatViewT<ChatView>
    {
    private:
        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer timer = nullptr;
        winrt::event_token timerEventToken;
        winrt::Windows::Foundation::Uri uri = nullptr;

    public:
        ChatView();
        ~ChatView();
        void ClosePopupButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ChatInput_PreviewKeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void ClearChatButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void UserControl_Unloaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        void DispatcherQueueTimer_Tick(winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer const& sender, winrt::Windows::Foundation::IInspectable const& args);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct ChatView : ChatViewT<ChatView, implementation::ChatView>
    {
    };
}
