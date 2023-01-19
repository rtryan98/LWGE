#pragma once

#include <mutex>

namespace lwge::thread
{
    template<bool Condition>
    class ConditionalLock;

    template<>
    class ConditionalLock<true>
    {
    public:
        ConditionalLock()
            : m_mutex()
        {
            m_mutex.lock();
        }

        ~ConditionalLock()
        {
            m_mutex.unlock();
        }

    private:
        std::mutex m_mutex;
    };

    template<>
    class ConditionalLock<false>
    {};
}
