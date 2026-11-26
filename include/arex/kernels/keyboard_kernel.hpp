#ifndef AREX_KERNELS_KEYBOARD_KERNEL_HPP
#define AREX_KERNELS_KEYBOARD_KERNEL_HPP

#include <cstdint>
#include <termios.h>

namespace arex::kernels {

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

class KeyboardKernel {
public:
    KeyboardKernel();
    ~KeyboardKernel();

    KeyboardKernel(const KeyboardKernel&) = delete;
    KeyboardKernel& operator=(const KeyboardKernel&) = delete;

    KeyboardKernel(KeyboardKernel&&) noexcept = default;
    KeyboardKernel& operator=(KeyboardKernel&&) noexcept = default;

    [[nodiscard]] KeyCode read_key_nonblocking();
    [[nodiscard]] KeyCode read_key_blocking();

private:
    void enable_raw_mode();
    void disable_raw_mode();

    struct termios _original_termios{};
    bool _raw_mode_enabled{false};
};

} // namespace arex::kernels

#endif // AREX_KERNELS_KEYBOARD_KERNEL_HPP
