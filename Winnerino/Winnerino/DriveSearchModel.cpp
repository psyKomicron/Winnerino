#include "pch.h"
#include "DriveSearchModel.h"
#if __has_include("DriveSearchModel.g.cpp")
#include "DriveSearchModel.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Data;

namespace winrt::Winnerino::implementation
{
    DriveSearchModel::DriveSearchModel(winrt::hstring driveName)
    {
        _driveName = driveName;
    }

    bool DriveSearchModel::IsSelected()
    {
        return _isSelected;
    }

    void DriveSearchModel::IsSelected(bool isSelected)
    {
        _isSelected = isSelected;
        propertyChanged(*this, PropertyChangedEventArgs{ L"IsSelected" });
    }

    winrt::event_token DriveSearchModel::PropertyChanged(PropertyChangedEventHandler const& value)
    {
        return propertyChanged.add(value);
    }

    void DriveSearchModel::PropertyChanged(winrt::event_token const& token)
    {
        propertyChanged.remove(token);
    }
}
