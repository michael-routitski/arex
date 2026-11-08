#include <arex/key_listener.hpp>

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

namespace arex {

KeyListener::KeyListener() {
    enable_raw_mode();
}

KeyListener::~KeyListener() {
    disable_raw_mode();
}

void KeyListener::enable_raw_mode() {
    if (m_raw_mode_enabled) return;

    if (tcgetattr(STDIN_FILENO, &m_original_termios) == -1) {
        return;
    }

    struct termios raw = m_original_termios;

    // Input flags
    raw.c_iflag &= static_cast<tcflag_t>(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));

    // Keep ONLCR so \n expands to \r\n automatically, or handle output cleanly
    raw.c_oflag |= static_cast<tcflag_t>(ONLCR);

    // Control flags
    raw.c_cflag |= static_cast<tcflag_t>(CS8);

    // Local flags: disable ECHO, ICANON, IEXTEN, ISIG
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON | IEXTEN | ISIG));

    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0) {
        m_raw_mode_enabled = true;
    }
}

void KeyListener::disable_raw_mode() {
    if (m_raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_original_termios);
        m_raw_mode_enabled = false;
    }
}

KeyCode KeyListener::read_key_nonblocking() {
    char ch = 0;
    auto bytes_read = ::read(STDIN_FILENO, &ch, 1);

    if (bytes_read <= 0) {
        return KeyCode::None;
    }

    switch (ch) {
        case 'w':
        case 'k': return KeyCode::Up;
        case 's':
        case 'j': return KeyCode::Down;
        case 'a':
        case 'h': return KeyCode::Left;
        case 'd':
        case 'l': return KeyCode::Right;
        case 'g': return KeyCode::Home;
        case 'G': return KeyCode::End;
        case 'q': return KeyCode::Escape;
        case '\n':
        case '\r': return KeyCode::Enter;
        case 127:
        case '\b': return KeyCode::Backspace;
        default: break;
    }

    // ANSI Escape Sequences (\x1b[A, \x1b[B, etc.)
    if (ch == '\x1b') {
        char seq[2];
        // Give the terminal a 1ms window to yield the rest of the escape sequence
        ::usleep(1000);

        if (::read(STDIN_FILENO, &seq[0], 1) <= 0) return KeyCode::Escape;
        if (::read(STDIN_FILENO, &seq[1], 1) <= 0) return KeyCode::Escape;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KeyCode::Up;
                case 'B': return KeyCode::Down;
                case 'C': return KeyCode::Right;
                case 'D': return KeyCode::Left;
                case 'H': return KeyCode::Home;
                case 'F': return KeyCode::End;
                default: break;
            }
        }
        return KeyCode::Escape;
    }

    return KeyCode::Unknown;
}

KeyCode KeyListener::read_key_blocking() {
    while (true) {
        if (const auto key = read_key_nonblocking(); key != KeyCode::None) {
            return key;
        }
        ::usleep(1000);
    }
}

} // namespace arex