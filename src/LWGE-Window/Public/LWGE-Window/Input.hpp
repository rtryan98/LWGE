#pragma once

#include <array>

namespace lwge
{
    enum class KeyCode : uint8_t;

    class Input
    {
    public:

        [[nodiscard]] bool is_key_pressed(KeyCode key) const;
        [[nodiscard]] bool is_key_released(KeyCode key) const;
        [[nodiscard]] bool is_key_clicked(KeyCode key) const;

    private:
        std::array<uint8_t, 256> m_keys_current_frame;
        std::array<uint8_t, 256> m_keys_last_frame;

        friend class Window;
    };
}
