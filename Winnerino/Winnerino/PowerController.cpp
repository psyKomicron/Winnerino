#include "pch.h"
#include "PowerController.h"
#include "windows.h"
#include "winuser.h"
#include "powrprof.h"

namespace winrt::Winnerino
{
	PowerController::PowerController(bool force, DWORD gracePeriod)
	{
		_force = force;
		this->gracePeriod = gracePeriod;
	}

#pragma region public
	bool PowerController::lock()
	{
		return LockWorkStation();
	}

	bool PowerController::shutdown()
	{
		if (!getPrivileges())
		{
			return false;
		}
		else
		{
			DWORD flags = SHUTDOWN_POWEROFF;
			if (_force)
			{
				flags |= SHUTDOWN_FORCE_SELF;
			}

			return InitiateShutdown(nullptr, nullptr, gracePeriod, flags, SHTDN_REASON_MINOR_MAINTENANCE);
		}
	}

	bool PowerController::sleep(bool hibernate, bool respect)
	{
		if (!checkHibernationAllowed())
		{
			if (respect) 
			{
				// Cannot hibernate the system
				return false;
			}
			else
			{
				hibernate = false;
			}
		}

		return SetSuspendState(hibernate, _force, false);
	}
#pragma endregion 

	bool PowerController::getPrivileges()
	{
		HANDLE tokenHandle;
		TOKEN_PRIVILEGES privs{};

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tokenHandle))
		{
			// Failed to open current process token.
			return false;
		}
		if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &privs.Privileges[0].Luid))
		{
			// Failed to lookup privilege value.
			return false;
		}

		privs.PrivilegeCount = 1;
		privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(tokenHandle, false, &privs, 0, (PTOKEN_PRIVILEGES)NULL, 0))
		{
			// Cannot adjust token privileges - Prob no permissions.
			return false;
		}

		// Check if function succeeded to make privileges changes
		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			// Not all privileges assigned - Can do a look to what has been assigned and return specific code
			return false;
		}

		return true;
	}

	bool PowerController::checkHibernationAllowed()
	{
		SYSTEM_POWER_CAPABILITIES capas;
		if (!GetPwrCapabilities(&capas))
		{
			// Failed to get power capabilities, assume hibernation is not supported.
			return false;
		}
		return capas.FastSystemS4 || capas.SystemS4;
	}
}
