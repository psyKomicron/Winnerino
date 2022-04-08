#include "pch.h"
#include "TestViewModel.h"
#if __has_include("TestViewModel.g.cpp")
#include "TestViewModel.g.cpp"
#endif

namespace winrt::Winnerino::implementation
{
    int32_t TestViewModel::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void TestViewModel::MyProperty(int32_t /*value*/)
    {
        throw hresult_not_implemented();
    }
}
