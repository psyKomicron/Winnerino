#pragma once

#include <pch.h>

namespace Winnerino::Storage 
{
	class DriveInfo
	{
	public:
		DriveInfo() = default;
		DriveInfo(const winrt::hstring& driveName, const winrt::hstring& volumeName, const bool& isConnected) :
			_driveName(driveName),
			_volumeName(volumeName),
			_isConnected(isConnected)
		{
		};

		bool IsConnected() const
		{
			return _isConnected;
		}
		winrt::hstring DriveName() const
		{
			return _driveName;
		};
		winrt::hstring VolumeName() const
		{
			return _volumeName;
		};

	private:
		bool _isConnected;
		winrt::hstring _driveName;
		winrt::hstring _volumeName;
	};
}

