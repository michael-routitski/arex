#include <arex/terminal_canvas.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>

#if defined(_WIN32)
    #include <io.h>
    #include <windows.h>
    #define AREX_WRITE(fd, buf, count) _write(fd, buf, static_cast<unsigned int>(count))
    #define STDOUT_FILENO 1
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #define AREX_WRITE(fd, buf, count) write(fd, buf, count)
#endif

namespace arex {

namespace {
constexpr std::string_view ANSI_ENTER_ALT_SCREEN = "\x1b[?1049h";
constexpr std::string_view ANSI_LEAVE_ALT_SCREEN = "\x1b[?1049l";
constexpr std::string_view ANSI_CURSOR_HOME       = "\x1b[H";
constexpr std::string_view ANSI_CLEAR_FROM_CURSOR = "\x1b[J";
constexpr std::string_view ANSI_HIDE_CURSOR       = "\x1b[?25l";
constexpr std::string_view ANSI_SHOW_CURSOR       = "\x1b[?25h";
constexpr std::string_view ANSI_CLEAR_LINE        = "\x1b[K";
constexpr std::string_view ANSI_RESET             = "\x1b[0m";
constexpr std::string_view ANSI_BOLD              = "\x1b[1m";
constexpr std::string_view ANSI_DIM               = "\x1b[2m";
constexpr std::string_view ANSI_CYAN              = "\x1b[36m";
constexpr std::string_view ANSI_GREEN             = "\x1b[32m";
constexpr std::string_view ANSI_YELLOW            = "\x1b[33m";
constexpr std::string_view ANSI_INVERSE           = "\x1b[7m";
} // namespace

TerminalCanvas::TerminalCanvas(std::size_t initial_buffer_capacity) {
    m_buffer.reserve(initial_buffer_capacity);
}

TerminalCanvas::~TerminalCanvas() {
    leave_alternate_screen();
}

void TerminalCanvas::enter_alternate_screen() {
    if (!m_in_alternate_screen) {
        m_buffer.clear();
        append_ansi(ANSI_ENTER_ALT_SCREEN);
        append_ansi(ANSI_HIDE_CURSOR);
        write_raw_buffer();
        m_in_alternate_screen = true;
    }
}

void TerminalCanvas::leave_alternate_screen() {
    if (m_in_alternate_screen) {
        m_buffer.clear();
        append_ansi(ANSI_SHOW_CURSOR);
        append_ansi(ANSI_LEAVE_ALT_SCREEN);
        write_raw_buffer();
        m_in_alternate_screen = false;
    }
}

TerminalSize TerminalCanvas::get_terminal_size() {
    TerminalSize size{24, 80};
#if !defined(_WIN32)
    struct winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0) {
        size.rows = ws.ws_row;
        size.cols = ws.ws_col;
    }
#else
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        size.rows = static_cast<std::size_t>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
        size.cols = static_cast<std::size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    }
#endif
    return size;
}

inline void TerminalCanvas::append_ansi(std::string_view code) {
    m_buffer.append(code);
}

inline void TerminalCanvas::clear_line_to_end() {
    m_buffer.append(ANSI_CLEAR_LINE);
}

void TerminalCanvas::render(const RenderState& state) {
    m_buffer.clear();
    append_ansi(ANSI_CURSOR_HOME);

    const TerminalSize term_size = get_terminal_size();

    // 1. Calculate Overhead Lines:
    // Header (1) + Preview Frame Top (1) + Preview Content (preview_height) + Preview Frame Bottom (1) + Status Footer (1)
    const std::size_t fixed_overhead = 1 + 1 + state.preview_height + 1 + 1;
    const std::size_t available_menu_rows = (term_size.rows > fixed_overhead) ? (term_size.rows - fixed_overhead) : 4;

    // 2. Render Header
    append_ansi(ANSI_BOLD);
    append_ansi(ANSI_CYAN);
    m_buffer.append(" [arex] ");
    m_buffer.append(state.current_path);
    append_ansi(ANSI_RESET);
    clear_line_to_end();
    m_buffer.append("\r\n");

    // 3. Render Preview Pane
    m_buffer.append("┌─ Preview ──────────────────────────────────────────");
    clear_line_to_end();
    m_buffer.append("\r\n");

    for (std::size_t i = 0; i < state.preview_height; ++i) {
        m_buffer.append("│ ");
        if (i < state.preview_lines.size()) {
            append_ansi(ANSI_DIM);
            m_buffer.append(state.preview_lines[i]);
            append_ansi(ANSI_RESET);
        }
        clear_line_to_end();
        m_buffer.append("\r\n");
    }

    m_buffer.append("└────────────────────────────────────────────────────");
    clear_line_to_end();
    m_buffer.append("\r\n");

    // 4. Compute Menu Viewport Slice
    const std::size_t total_items = state.menu_items.size();
    std::size_t start_idx = 0;
    std::size_t end_idx = total_items;

    if (total_items > available_menu_rows) {
        // Keep selected_index in view
        if (state.selected_index >= available_menu_rows) {
            start_idx = state.selected_index - available_menu_rows + 1;
        }
        end_idx = std::min(total_items, start_idx + available_menu_rows);
    }

    // 5. Render Scrolled Menu Subset
    for (std::size_t i = start_idx; i < end_idx; ++i) {
        const auto& item = state.menu_items[i];
        const bool is_selected = (i == state.selected_index);

        // Indicate hidden items above/below using "..." on boundary items
        const bool is_first_visible = (i == start_idx && start_idx > 0);
        const bool is_last_visible = (i == end_idx - 1 && end_idx < total_items);

        if (is_selected) {
            append_ansi(ANSI_INVERSE);
            append_ansi(ANSI_BOLD);
            m_buffer.append(" > ");
        } else {
            m_buffer.append("   ");
        }

        if (is_first_visible || is_last_visible) {
            append_ansi(ANSI_DIM);
            m_buffer.append("... [ ");
            m_buffer.append(item.name);
            m_buffer.append(" ]");
        } else {
            switch (item.kind) {
                case ItemKind::Action: {
                    append_ansi(ANSI_YELLOW);
                    m_buffer.append("[ACTION] ");
                    m_buffer.append(item.name);
                    break;
                }
                case ItemKind::Directory: {
                    append_ansi(ANSI_CYAN);
                    m_buffer.append("[DIR]    ");
                    m_buffer.append(item.name);
                    m_buffer.push_back('/');
                    break;
                }
                case ItemKind::File:
                default: {
                    append_ansi(ANSI_GREEN);
                    m_buffer.append("[FILE]   ");
                    m_buffer.append(item.name);
                    break;
                }
            }
        }

        append_ansi(ANSI_RESET);
        clear_line_to_end();
        m_buffer.append("\r\n");
    }

    // 6. Render Status Bar
    if (!state.status_message.empty()) {
        append_ansi(ANSI_DIM);
        m_buffer.append(" -- ");
        m_buffer.append(state.status_message);
        append_ansi(ANSI_RESET);
        clear_line_to_end();
        m_buffer.append("\r\n");
    }

    // 7. Clear remaining screen real-estate to bottom of terminal
    append_ansi(ANSI_CLEAR_FROM_CURSOR);
    write_raw_buffer();
}

void TerminalCanvas::write_raw_buffer() {
    if (m_buffer.empty()) return;

    std::size_t total_written = 0;
    const char* data = m_buffer.data();
    const std::size_t length = m_buffer.size();

    while (total_written < length) {
        const auto bytes = AREX_WRITE(
            STDOUT_FILENO,
            data + total_written,
            length - total_written
        );

        if (bytes <= 0) {
            break;
        }
        total_written += static_cast<std::size_t>(bytes);
    }
}

} // namespace arex