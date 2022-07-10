#pragma once
#include <string>

using namespace winrt;

namespace Winnerino::Storage
{
    class DirectorySizeCalculator
    {
    public:
        DirectorySizeCalculator() = default;

        event_token Progress(Windows::Foundation::TypedEventHandler<Windows::Foundation::IInspectable, Windows::Foundation::IReference<uint_fast64_t>> const& handler)
        {
            return m_event.add(handler);
        };
        void Progress(event_token const& token)
        {
            m_event.remove(token);
        };

        uint_fast64_t GetSize(hstring const& path, bool parallelize = false);

    private:
        winrt::event<Windows::Foundation::TypedEventHandler<Windows::Foundation::IInspectable, Windows::Foundation::IReference<uint_fast64_t>>> m_event;

        inline void RaiseProgress(uint_fast64_t newSize);
    };
}

