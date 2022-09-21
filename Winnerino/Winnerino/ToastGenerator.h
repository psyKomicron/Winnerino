#pragma once
namespace Winnerino::Common
{
	class ToastGenerator
	{
	public:
		ToastGenerator() = default;

		void ShowToastNotification(winrt::hstring const& text);

	private:
		inline bool AreNotificationsAllowed();
	};
}
