#include "pch.h"
#include "DriveViewModel.h"
#if __has_include("DriveViewModel.g.cpp")
#include "DriveViewModel.g.cpp"
#endif

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml::Data;

namespace winrt::Winnerino::implementation
{
    inline winrt::hstring DriveViewModel::Name() const
    {
        return _name;
    }

    inline void DriveViewModel::Name(const hstring& value)
    {
        _name = value;
    }
    
    inline winrt::hstring DriveViewModel::FriendlyName() const
    {
        return _friendlyName;
    }
    
    inline void DriveViewModel::FriendlyName(const winrt::hstring& value)
    {
        _friendlyName = value;
    }

    winrt::hstring DriveViewModel::ToString() const
    {
        if (_friendlyName.empty()) 
        {
            return _name;
        }
        else 
        {
            return _friendlyName + L"(" + _name + L")";
        }
    }
}
