#pragma once

#include "LWGE-RenderDriver/Resource.hpp"

#include <LWGE-Common/Handle.hpp>
#include <LWGE-Thread/ConditionalLock.hpp>
#include <span>
#include <vector>

namespace lwge::rd
{
    template<class T, std::size_t Size, bool UseMutex>
    class HandleResourcePool
    {
        static_assert(Size < UINT32_MAX);
        using HandleType = Handle<T, HandleValueType>;
        constexpr static uint16_t GENERATED_FLAG = uint16_t(0x8000);
        constexpr static uint32_t NO_HEAD = ~0u;

    public:
        HandleResourcePool() noexcept
        {
            m_data.resize(Size);
        }

        [[nodiscard]] HandleType insert(uint16_t flags) noexcept
        {
            thread::ConditionalLock<UseMutex> lock;
            uint32_t idx = 0;
            if (m_head != NO_HEAD)
            {
                idx = m_head;
                m_head = m_data.at(idx).next;
            }
            else
            {
                m_data.push_back({});
                idx = uint32_t(m_data.size() - 1);
            }
            auto& element = m_data[idx];
            element.stored = {};
            element.flags = uint16_t(GENERATED_FLAG | flags);
            auto handle = HandleType({
                .flags = uint16_t(GENERATED_FLAG | flags),
                .gen = element.gen,
                .idx = idx
                });
            return handle;
        }

        [[nodiscard]] std::vector<HandleType> insert_bulk(
            std::span<uint16_t> flags_view) noexcept
        {
            thread::ConditionalLock<UseMutex> lock;
            std::vector<HandleType> result;
            result.reserve(flags_view.size());
            for (auto flags : flags_view)
            {
                result.push_back(insert(flags));
            }
            return result;
        }

        void remove(HandleType handle) noexcept
        {
            thread::ConditionalLock<UseMutex> lock;
            auto hv = handle.get_underlying_value();
            auto& element = m_data[hv.idx];
            element.flags = 0;
            element.gen++;
            element.stored = {};
            m_head = hv.idx;
        }

        void remove_bulk(std::span<HandleType> handles) noexcept
        {
            thread::ConditionalLock<UseMutex> lock;
            for (auto handle : handles)
            {
                remove(handle);
            }
        }

        /// @brief Same conditions as operator[].
        [[nodiscard]] bool valid(HandleType handle) const noexcept
        {
            auto hv = handle.get_underlying_value();
            const auto& element =  m_data.at(hv.idx);
            return (element.gen == hv.gen)
                && (element.flags == hv.flags)
                && (element.flags & GENERATED_FLAG)
                && (hv.flags & GENERATED_FLAG);
        }

        /// @details This function is only thread-safe if the condition
        /// holds that any destroyed handle is no longer accessed after
        /// its *deferred* destruction. If this handle pool would be
        /// used in a non-deferred context, there'd be a race condition
        /// if one thread deletes the handle and another one reads it.
        /// However, as long as a fork-join approach is used and the
        /// actual handles are destroyed after MAX_CONCURRENT_GPU_FRAMES,
        /// no race condition can appear unless there is a rogue
        /// thread destroying the given handle.
        [[nodiscard]] T& operator[](HandleType handle) noexcept
        {
            auto hv = handle.get_underlying_value();
            return m_data[hv.idx].stored;
        }

        /// @brief Same conditions as operator[].
        [[nodiscard]] const T& at(HandleType handle) const noexcept
        {
            auto hv = handle.get_underlying_value();
            return m_data[hv.idx].stored;
        }

    private:
        uint32_t m_head = NO_HEAD;

        struct Contained
        {
            uint16_t flags;
            uint16_t gen;
            uint32_t next;
            T stored;
        };
        std::vector<Contained> m_data = {};
    };
}
