#pragma once

#include "NavigationViewItemTag.g.h"

namespace winrt::Winnerino::implementation
{
    struct NavigationViewItemTag : NavigationViewItemTagT<NavigationViewItemTag>
    {
    public:
        NavigationViewItemTag() = default;

        winrt::hstring Action() const;
        void Action(const winrt::hstring& value);
        winrt::Windows::Foundation::IInspectable Data() const;
        void Data(const winrt::Windows::Foundation::IInspectable& value);

    private:
        winrt::hstring _action;
        winrt::Windows::Foundation::IInspectable _data;
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct NavigationViewItemTag : NavigationViewItemTagT<NavigationViewItemTag, implementation::NavigationViewItemTag>
    {
    };
}
