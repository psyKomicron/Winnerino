#pragma once
namespace winrt::Winnerino
{
	class PowerController
	{
	private:
		bool _force;
		DWORD gracePeriod;

		bool getPrivileges();
		bool checkHibernationAllowed();

	public:
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="force">true to force power functions actions.</param>
		/// <param name="gracePeriod"><param>
		PowerController(bool force, DWORD gracePeriod = 0);

		/// <summary>
		/// Locks the local computer.
		/// </summary>
		/// <returns>Returns true if the system successfully locked.</returns>
		bool lock();
		/// <summary>
		/// Shutdowns the local computer.
		/// </summary>
		/// <returns>true if the system accepted the shutdown request.</returns>
		bool shutdown();
		/// <summary>
		/// Sets the computer to sleep (or hibernate) S4/S5.
		/// </summary>
		/// <param name="hibernate">true to hibernate the computer, false to put the computer to sleep.</param>
		/// <returns>True if the system accepted the request.</returns>
		bool sleep(bool hibernate = false, bool force = false);
	};
}

