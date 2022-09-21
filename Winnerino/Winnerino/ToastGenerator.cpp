#include "pch.h"
#include "ToastGenerator.h"

using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::Data::Xml::Dom;

namespace Winnerino::Common
{
	void ToastGenerator::ShowToastNotification(winrt::hstring const& message)
	{
        if (AreNotificationsAllowed())
        {
            XmlDocument toastContent{};
            XmlElement root = toastContent.CreateElement(L"toast");
            toastContent.AppendChild(root);

            XmlElement visual = toastContent.CreateElement(L"visual");
            root.AppendChild(visual);

            XmlElement binding = toastContent.CreateElement(L"binding");
            binding.SetAttribute(L"template", L"ToastText01");
            visual.AppendChild(binding);

            XmlElement text = toastContent.CreateElement(L"text");
            text.SetAttribute(L"id", L"1");
            text.InnerText(message);
            binding.AppendChild(text);

            ToastNotification toastNotif{ toastContent };
            ToastNotificationManager::CreateToastNotifier().Show(toastNotif);
        }
	}

    bool ToastGenerator::AreNotificationsAllowed()
    {
        return winrt::unbox_value_or<bool>(winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values().TryLookup(L"NotificationsEnabled"), true);
    }
}