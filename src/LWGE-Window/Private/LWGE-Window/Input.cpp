#include "LWGE-Window/Input.hpp"

namespace lwge
{
    constexpr uint8_t high_bit(uint8_t in)
    {
        return in & 0x80;
    }

    bool Input::is_key_pressed(KeyCode key) const
    {
        return high_bit(m_keys_current_frame[uint8_t(key)]);
    }

    bool Input::is_key_released(KeyCode key) const
    {
        return high_bit(m_keys_last_frame[uint8_t(key)])
            && !high_bit(m_keys_current_frame[uint8_t(key)]);
    }

    bool Input::is_key_clicked(KeyCode key) const
    {
        return !high_bit(m_keys_last_frame[uint8_t(key)])
            && high_bit(m_keys_current_frame[uint8_t(key)]);
    }
}
