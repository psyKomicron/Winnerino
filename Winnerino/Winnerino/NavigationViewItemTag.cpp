#include "pch.h"
#include "NavigationViewItemTag.h"
#if __has_include("NavigationViewItemTag.g.cpp")
#include "NavigationViewItemTag.g.cpp"
#endif

namespace winrt::Winnerino::implementation
{
    winrt::hstring NavigationViewItemTag::Action() const
    {
        return _action;
    }

    void NavigationViewItemTag::Action(const winrt::hstring& value)
    {
        _action = value;
    }

    winrt::Windows::Foundation::IInspectable NavigationViewItemTag::Data() const
    {
        return _data;
    }

    void NavigationViewItemTag::Data(const winrt::Windows::Foundation::IInspectable& value)
    {
        _data = value;
    }
}
