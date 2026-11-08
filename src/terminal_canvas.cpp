#include <arex/terminal_canvas.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <iostream>

#if defined(_WIN32)
    #include <io.h>
    #define AREX_WRITE(fd, buf, count) _write(fd, buf, static_cast<unsigned int>(count))
    #define STDOUT_FILENO 1
#else
    #include <unistd.h>
    #define AREX_WRITE(fd, buf, count) write(fd, buf, count)
#endif

namespace arex {

namespace {
// ANSI Control Sequences
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

void draw_header(std::string& buffer, const RenderState& state) {
    buffer.append(ANSI_BOLD);
    buffer.append(ANSI_CYAN);
    buffer.append(" [arex] ");
    buffer.append(state.current_path);
    buffer.append(ANSI_RESET);
    buffer.append(ANSI_CLEAR_LINE);
    buffer.append("\r\n");
}

void draw_file_preview_pane(std::string& buffer, const RenderState& state) {
    buffer.append("┌─ Preview ──────────────────────────────────────────");
    buffer.append(ANSI_CLEAR_LINE);
    buffer.append("\r\n");

    for (std::size_t i = 0; i < state.preview_height; ++i) {
        buffer.append("│ ");
        if (i < state.preview_lines.size()) {
            buffer.append(ANSI_DIM);
            buffer.append(state.preview_lines[i]);
            buffer.append(ANSI_RESET);
        }
        buffer.append(ANSI_CLEAR_LINE);
        buffer.append("\r\n");
    }

    buffer.append("└────────────────────────────────────────────────────");
    buffer.append(ANSI_CLEAR_LINE);
    buffer.append("\r\n");
}

void draw_menu_items(std::string& buffer, const RenderState& state) {
    for (std::size_t i = 0; i < state.menu_items.size(); ++i) {
        const auto& item = state.menu_items[i];
        const bool is_selected = (i == state.selected_index);

        if (is_selected) {
            buffer.append(ANSI_INVERSE);
            buffer.append(ANSI_BOLD);
            buffer.append(" > ");
        } else {
            buffer.append("   ");
        }

        switch (item.kind) {
            case ItemKind::Action: {
                buffer.append(ANSI_YELLOW);
                buffer.append("[ACTION] ");
                buffer.append(item.name);
                break;
            }
            case ItemKind::Directory: {
                buffer.append(ANSI_CYAN);
                buffer.append("[DIR]    ");
                buffer.append(item.name);
                buffer.push_back('/');
                break;
            }
            case ItemKind::File:
            default: {
                buffer.append(ANSI_GREEN);
                buffer.append("[FILE]   ");
                buffer.append(item.name);
                break;
            }
        }

        buffer.append(ANSI_RESET);
        buffer.append(ANSI_CLEAR_LINE);
        buffer.append("\r\n");
    }
}

void draw_status_bar(std::string& buffer, const RenderState& state) {
    if (!state.status_message.empty()) {
        buffer.append(ANSI_DIM);
        buffer.append(" -- ");
        buffer.append(state.status_message);
        buffer.append(ANSI_RESET);
        buffer.append(ANSI_CLEAR_LINE);
        buffer.append("\r\n");
    }
}
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

inline void TerminalCanvas::append_ansi(std::string_view code) {
    m_buffer.append(code);
}

inline void TerminalCanvas::clear_line_to_end() {
    m_buffer.append(ANSI_CLEAR_LINE);
}

void TerminalCanvas::render(const RenderState& state) {
    m_buffer.clear();

    // reset cursor to top-left corner for full terminal redraw
    append_ansi(ANSI_CURSOR_HOME);

    draw_header(m_buffer, state);
    draw_file_preview_pane(m_buffer, state);
    draw_menu_items(m_buffer, state);
    draw_status_bar(m_buffer, state);

    // clear any trailing content below the current frame in one step
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