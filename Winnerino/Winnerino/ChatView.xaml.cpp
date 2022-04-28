#include "pch.h"
#include "ChatView.xaml.h"
#if __has_include("ChatView.g.cpp")
#include "ChatView.g.cpp"
#endif
using namespace std::chrono;
using namespace std::chrono_literals;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Microsoft::UI::Xaml::Documents;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace winrt::Windows::Foundation;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    ChatView::ChatView()
    {
        InitializeComponent();
        InfoPopup().XamlRoot(MainGrid().XamlRoot());

        timer = DispatcherQueue().CreateTimer();
        timer.Interval(3s);
        timer.IsRepeating(false);
        timerEventToken = timer.Tick({ this, &ChatView::DispatcherQueueTimer_Tick });
        timer.Start();
    }

    ChatView::~ChatView()
    {
        timer.Tick(timerEventToken);
    }

    void ChatView::ClosePopupButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        InfoPopup().IsOpen(false);
    }

    void ChatView::ChatInput_PreviewKeyDown(IInspectable const&, KeyRoutedEventArgs const& args)
    {
        if (!args.Handled() && args.Key() == Windows::System::VirtualKey::Enter)
        {
            RichTextBlock block{};
            block.TextWrapping(TextWrapping::Wrap);
            block.IsTextSelectionEnabled(true);

            Paragraph p{};
            Run r{};

            InlineUIContainer imageContainer{};

            imageContainer.KeyTipPlacementMode(KeyTipPlacementMode::Center);
            Image pepeMeltdown{};
            pepeMeltdown.Height(60);
            BitmapImage bitmapImage{};
            if (!uri)
            {
                uri = Uri{ L"E:\\julie\\Pictures\\bttv\\pepeMeltdown.gif" };
            }
            bitmapImage.UriSource(uri);
            pepeMeltdown.Source(bitmapImage);

            imageContainer.Child(pepeMeltdown);

            r.Text(ChatInput().Text());
            p.Inlines().Append(r);

            p.Inlines().Append(imageContainer);

            block.Blocks().Append(p);

            ListViewItem item{};
            //item.IsSelected(true);
            item.Content(block);

            ChatListView().Items().Append(item);

            InfoTextBlock().Text(to_hstring(ChatListView().Items().Size()));
        }
    }

    void ChatView::ClearChatButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        ChatListView().Items().Clear();
    }

    void ChatView::DispatcherQueueTimer_Tick(DispatcherQueueTimer const&, IInspectable const&)
    {
        InfoPopup().IsOpen(true);
    }

    void ChatView::UserControl_Unloaded(IInspectable const&, RoutedEventArgs const&)
    {
        ChatListView().Items().Clear();
    }
}