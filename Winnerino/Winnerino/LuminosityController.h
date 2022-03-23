#pragma once
namespace winrt::Winnerino::Devices
{
	class LuminosityController
	{
	private:
		double oldValue;

	public:
		LuminosityController() = default;

		bool setLuminosity(double value);
	};
}