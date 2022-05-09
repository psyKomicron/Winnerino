#pragma once
#include <string>

namespace winrt::Winnerino::Storage
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

        uint_fast64_t getSize(hstring const& path, bool parallelize = false);

    private:
        event<Windows::Foundation::TypedEventHandler<Windows::Foundation::IInspectable, Windows::Foundation::IReference<uint_fast64_t>>> m_event;

        inline uint_fast64_t convertSize(DWORD const& high, DWORD const& low);
        inline void raiseProgress(uint_fast64_t newSize);
    };
}

