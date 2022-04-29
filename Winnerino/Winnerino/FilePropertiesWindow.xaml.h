#pragma once

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include "FilePropertiesWindow.g.h"

#pragma pop_macro("GetCurrentTime")

namespace winrt::Winnerino::implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow>
    {
    private:
        Microsoft::UI::Windowing::AppWindow appWindow = nullptr;

    public:
        FilePropertiesWindow();
        ~FilePropertiesWindow();

    private:
        void InitWindow();
    };
}

namespace winrt::Winnerino::factory_implementation
{
    struct FilePropertiesWindow : FilePropertiesWindowT<FilePropertiesWindow, implementation::FilePropertiesWindow>
    {
    };
}
