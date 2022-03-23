#include "pch.h"
#include "LuminosityController.h"
#include "winuser.h"
using namespace std;

namespace winrt::Winnerino::Devices
{
	bool LuminosityController::setLuminosity(double value)
	{
		if (value != oldValue)
		{
			oldValue = value;
			DWORD deviceNumber = 0;
			MONITORENUMPROC Monitorenumproc;
			DISPLAY_DEVICE displayDevice = DISPLAY_DEVICE();
			vector<DISPLAY_DEVICE> displayDevices = vector<DISPLAY_DEVICE>();

			while (EnumDisplayDevices(NULL, deviceNumber, &displayDevice, 0))
			{

			}
		}

		// not implemented
		return false;
	}
}