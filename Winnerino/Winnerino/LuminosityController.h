#pragma once
#include "physicalmonitorenumerationapi.h"

namespace winrt::Winnerino::Controllers
{
	class LuminosityController
	{
	private:
		double oldValue = 0;
		bool isOnBoardDisplay;

		void destroyPhysicalMonitor(DWORD size, LPPHYSICAL_MONITOR monitors);
		void initializeOnBoardDisplayController();

	public:
		LuminosityController(bool isOnBoardDisplay);
		~LuminosityController();

		/// <summary>
		/// Must be called before doing any actions on the controller.
		/// </summary>
		void initializeController();
		bool setLuminosity(double value);
	};
}