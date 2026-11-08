#ifndef AREX_KEY_LISTENER_HPP
#define AREX_KEY_LISTENER_HPP

#include <cstdint>
#include <termios.h>

namespace arex {

enum class KeyCode : std::uint8_t {
    None = 0,
    Up,
    Down,
    Left,
    Right,
    Enter,
    Escape,
    Backspace,
    Home,
    End,
    Unknown
};

class KeyListener {
public:
    KeyListener();
    ~KeyListener();

    KeyListener(const KeyListener&) = delete;
    KeyListener& operator=(const KeyListener&) = delete;

    KeyListener(KeyListener&&) noexcept = default;
    KeyListener& operator=(KeyListener&&) noexcept = default;

    [[nodiscard]] KeyCode read_key_nonblocking();

    [[nodiscard]] KeyCode read_key_blocking();

private:
    void enable_raw_mode();
    void disable_raw_mode();

    struct termios m_original_termios{};
    bool m_raw_mode_enabled{false};
};

} // namespace arex

#endif // AREX_KEY_LISTENER_HPP