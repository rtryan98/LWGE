#pragma once

#include <cstdint>

namespace lwge
{
    template<typename T>
    concept HandleValueConcept = requires(T t1, T t2)
    {
        t1.idx;
        t1.flags;
        t1.gen;
        t1 == t2;
        t1 != t2;
    };

    struct DefaultHandleValueType
    {
        uint32_t flags : 8;
        uint32_t gen : 24;
        uint32_t idx;

        [[nodiscard]] bool operator==(DefaultHandleValueType other) noexcept
        {
            return (flags = other.flags) && (gen == other.gen) && (idx == other.idx);
        };
        [[nodiscard]] bool operator!=(DefaultHandleValueType other) noexcept
        {
            return !(*this == other);
        };
    };

    template<class ID, HandleValueConcept HV>
    class Handle
    {
    public:
        Handle() = default;
        explicit Handle(HV hv) : m_value(hv) {}
        explicit operator HV() const { return m_value; }

        friend bool operator==(Handle a, Handle b) { return a.m_value == b.m_value; }
        friend bool operator!=(Handle a, Handle b) { return !(a == b); }

    private:
        HV m_value;
    };
}
