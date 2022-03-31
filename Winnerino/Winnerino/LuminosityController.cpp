#include "pch.h"
#include "LuminosityController.h"
#include "winuser.h"
#include "highlevelmonitorconfigurationapi.h"

using namespace std;

#define IOCTL_VIDEO_QUERY_SUPPORTED_BRIGHTNESS \
  CTL_CODE(FILE_DEVICE_VIDEO, 0x125, METHOD_BUFFERED, FILE_ANY_ACCESS)

namespace winrt::Winnerino::Controllers
{
	LuminosityController::LuminosityController(bool isOnBoardDisplay)
	{
		this->isOnBoardDisplay = isOnBoardDisplay;
	}

	LuminosityController::~LuminosityController()
	{
		// for now do nothing
	}

	BOOL callback(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam)
	{
		vector<HMONITOR>* monitors = (vector<HMONITOR>*)lParam;
		monitors->push_back(hMonitor);
		return true; // keep enumerating
	}

	bool LuminosityController::setLuminosity(double value)
	{
		if (value != oldValue)
		{
			oldValue = value;
		}

		// not implemented
		return false;
	}

	void LuminosityController::initializeController()
	{
		if (isOnBoardDisplay)
		{
			initializeOnBoardDisplayController();
		}
		else
		{
			vector<HMONITOR> monitors = vector<HMONITOR>();
			EnumDisplayMonitors(NULL, NULL, callback, (LPARAM)&monitors);

			for (HMONITOR monitorHandle : monitors)
			{
				LPPHYSICAL_MONITOR monitorArray = NULL;
				DWORD monitorArraySize = NULL;
				// initialize variables
				if (!GetNumberOfPhysicalMonitorsFromHMONITOR(monitorHandle, &monitorArraySize))
				{
					// throw init error
					return;
				}
				monitorArray = (LPPHYSICAL_MONITOR)malloc(monitorArraySize * sizeof(PHYSICAL_MONITOR));
				if (monitorArray == NULL)
				{
					return; // throw init error
				}
				if (!GetPhysicalMonitorsFromHMONITOR(monitorHandle, monitorArraySize, monitorArray))
				{
					OutputDebugString(L"ERROR: GetPhysicalMonitorsFromHMONITOR failed.");
					free(monitorArray);
					return; // throw init error
				}

				DWORD monitorCapabilities = NULL;
				DWORD supportedColors = NULL;
				if (!GetMonitorCapabilities(monitorArray->hPhysicalMonitor, &monitorCapabilities, &supportedColors))
				{
					destroyPhysicalMonitor(monitorArraySize, monitorArray);

					// function failed
					OutputDebugString(L"ERROR: GetMonitorCapabilities failed.");

					// throw init error
					throw HRESULT_FROM_WIN32(GetLastError());
				}
				// test if monitor supports brightness control
				else if (monitorCapabilities && monitorCapabilities & MC_CAPS_BRIGHTNESS)
				{
					// we can change brightness for this monitor, add it to the (list of) monitors for future setLuminosity calls.
					DWORD minBrightness = 0, currentBrightness = 0, maxBrightness = 0;
					if (!GetMonitorBrightness(monitorArray->hPhysicalMonitor, &minBrightness, &currentBrightness, &maxBrightness))
					{
						destroyPhysicalMonitor(monitorArraySize, monitorArray);

						OutputDebugString(L"GetMonitorBrightness failed.");
						throw HRESULT_FROM_WIN32(GetLastError());
					}

					OutputDebugString(monitorArray->szPhysicalMonitorDescription);
				}

#ifdef _DEBUG
				destroyPhysicalMonitor(monitorArraySize, monitorArray);
#else
				else
				{
					// clean up
					destroyPhysicalMonitor(monitorArraySize, monitorArray);
					OutputDebugString(L"Cannot create LuminosityController, no monitors supports brightness level control.");
				}
#endif // DEBUG

			}
		}
	}

	void LuminosityController::initializeOnBoardDisplayController()
	{
		vector<DISPLAY_DEVICE> displayDevices = vector<DISPLAY_DEVICE>();

		DWORD devNum = 0;
		DISPLAY_DEVICE device = DISPLAY_DEVICE();
		device.cb = sizeof(DISPLAY_DEVICE);
		while (EnumDisplayDevices(NULL, devNum, &device, 0))
		{
			displayDevices.push_back(device);
			devNum++;
		}

		for (auto& displayDevice : displayDevices)
		{
			HANDLE monitorHandle = CreateFile(displayDevice.DeviceName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (monitorHandle != INVALID_HANDLE_VALUE)
			{
				char* buffer[256]{};
				DWORD bytesReturned = 0;
				if (DeviceIoControl(monitorHandle, IOCTL_VIDEO_QUERY_SUPPORTED_BRIGHTNESS, NULL, 0, buffer, 256, &bytesReturned, NULL))
				{
					// handle retreived data
					for (size_t i = 0; i < bytesReturned; i++)
					{
						OutputDebugString(to_hstring(buffer[i]).c_str());
						OutputDebugString(L"\n");
					}
				}
				else
				{
					OutputDebugString(L"ERROR: DeviceIoControl failed.");
					throw HRESULT_FROM_WIN32(GetLastError());
				}
			}
			else
			{
				OutputDebugString(L"Could not open display device. ");
				auto message = system_category().message(GetLastError());
				OutputDebugString(to_hstring(message).c_str());
				OutputDebugString(L"\n");
				//throw HRESULT_FROM_WIN32(GetLastError());
			}
		}
	}

	void LuminosityController::destroyPhysicalMonitor(DWORD size, LPPHYSICAL_MONITOR monitors)
	{
		DestroyPhysicalMonitors(size, monitors);
		free(monitors);
	}
}