#pragma once

#include "TestViewModel.g.h"

namespace winrt::Winnerino::implementation
{
    struct TestViewModel : TestViewModelT<TestViewModel>
    {
        TestViewModel() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct TestViewModel : TestViewModelT<TestViewModel, implementation::TestViewModel>
    {
    };
}
