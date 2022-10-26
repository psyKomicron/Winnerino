#include "pch.h"
#include "DriveView.h"
#if __has_include("DriveView.g.cpp")
#include "DriveView.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Winnerino::implementation
{
    DependencyProperty DriveView::_driveNameProperty = DependencyProperty::Register(
        L"DriveName",
        xaml_typename<hstring>(),
        xaml_typename<Winnerino::DriveView>(),
        PropertyMetadata(box_value(L"oioi"))
    );
    DependencyProperty DriveView::_friendlyNameProperty = DependencyProperty::Register(
        L"FriendlyName",
        xaml_typename<hstring>(),
        xaml_typename<Winnerino::DriveView>(),
        PropertyMetadata(box_value(L"segz"))
    );


    DriveView::DriveView()
    {
        DefaultStyleKey(winrt::box_value(L"Winnerino.DriveView"));
    }


    inline winrt::hstring DriveView::DriveName() const
    {
        return unbox_value<hstring>(GetValue(_driveNameProperty));
    }

    inline void DriveView::DriveName(const winrt::hstring& value)
    {
        SetValue(_driveNameProperty, box_value(value));
    }

    inline winrt::hstring DriveView::FriendlyName() const
    {
        return unbox_value<hstring>(GetValue(_friendlyNameProperty));
    }

    inline void DriveView::FriendlyName(const winrt::hstring& value)
    {
        SetValue(_friendlyNameProperty, box_value(value));
    }
}
