#include <arex/kernels/terminal_kernel.hpp>

#include <algorithm>
#include <cstring>

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

namespace arex::kernels {

TerminalSize TerminalKernel::get_terminal_size() {
    TerminalSize size;
#if !defined(_WIN32)
    struct winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0 && ws.ws_col > 0) {
        size.rows = static_cast<std::size_t>(ws.ws_row);
        size.cols = static_cast<std::size_t>(ws.ws_col);
    }
#else
    CONSOLE_SCREEN_BUFFER_INFO csbi{};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        size.rows = static_cast<std::size_t>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
        size.cols = static_cast<std::size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    }
#endif
    return size;
}

void TerminalKernel::append_output(std::string_view contents) {
    std::size_t offset = 0;
    while (offset < contents.size()) {
        if (_output_size == _output_buffer.size()) {
            flush_output();
        }

        const std::size_t remaining = _output_buffer.size() - _output_size;
        const std::size_t copy_count = std::min(contents.size() - offset, remaining);
        std::memcpy(_output_buffer.data() + _output_size, contents.data() + offset, copy_count);
        _output_size += copy_count;
        offset += copy_count;
    }
}

void TerminalKernel::append_output(const char* contents, std::size_t size) {
    if (size != 0 && contents != nullptr) {
        append_output(std::string_view(contents, size));
    }
}

void TerminalKernel::append_middle_truncated(
    std::string_view contents,
    std::size_t maximum_width
) {
    if (maximum_width == 0) {
        return;
    }
    if (contents.size() <= maximum_width) {
        append_output(contents);
        return;
    }
    if (maximum_width < 3) {
        if (maximum_width == 1) {
            append_output(".");
        } else {
            append_output("..");
        }
        return;
    }

    const std::size_t available = maximum_width - 3;
    std::size_t left_width = available / 2;
    std::size_t right_width = available - left_width;

    while (left_width > 0 && left_width < contents.size() &&
           (static_cast<unsigned char>(contents[left_width]) & 0xC0U) == 0x80U) {
        --left_width;
        ++right_width;
    }
    while (right_width > 0 &&
           (static_cast<unsigned char>(contents[contents.size() - right_width]) & 0xC0U) == 0x80U) {
        --right_width;
        ++left_width;
    }

    append_output(contents.substr(0, left_width));
    append_output("...");
    append_output(contents.substr(contents.size() - right_width, right_width));
}

void TerminalKernel::append_brightness(TerminalBrightness brightness) {
    switch (brightness) {
        case TerminalBrightness::Highlight:
            append_output(terminal_constants::ANSI_INVERSE);
            append_output(terminal_constants::ANSI_BOLD);
            break;
        case TerminalBrightness::Dim:
            append_output(terminal_constants::ANSI_DIM);
            append_output(terminal_constants::ANSI_DIM_TEXT);
            break;
        case TerminalBrightness::Dimmer:
            append_output(terminal_constants::ANSI_DIM);
            append_output(terminal_constants::ANSI_DIMMER_TEXT);
            break;
        case TerminalBrightness::Default:
            break;
    }
}

void TerminalKernel::append_menu_brightness(
    TerminalBrightness brightness,
    std::string_view normal,
    std::string_view dim,
    std::string_view dimmer
) {
    switch (brightness) {
        case TerminalBrightness::Highlight:
            append_output(terminal_constants::ANSI_INVERSE);
            append_output(terminal_constants::ANSI_BOLD);
            append_output(normal);
            break;
        case TerminalBrightness::Dim:
            append_output(dim);
            break;
        case TerminalBrightness::Dimmer:
            append_output(dimmer);
            break;
        case TerminalBrightness::Default:
            append_output(normal);
            break;
    }
}

void TerminalKernel::flush_output() {
    if (_output_size == 0) {
        return;
    }

    std::size_t total_written = 0;
    while (total_written < _output_size) {
        const auto bytes = AREX_WRITE(
            STDOUT_FILENO,
            _output_buffer.data() + total_written,
            _output_size - total_written
        );
        if (bytes <= 0) {
            break;
        }
        total_written += static_cast<std::size_t>(bytes);
    }
    _output_size = 0;
}

void TerminalKernel::enter_alternate_screen() {
    if (!_in_alternate_screen) {
        append_output(terminal_constants::ANSI_ENTER_ALT_SCREEN);
        append_output(terminal_constants::ANSI_HIDE_CURSOR);
        flush_output();
        _in_alternate_screen = true;
    }
}

void TerminalKernel::leave_alternate_screen() {
    if (_in_alternate_screen) {
        append_output(terminal_constants::ANSI_SHOW_CURSOR);
        append_output(terminal_constants::ANSI_LEAVE_ALT_SCREEN);
        flush_output();
        _in_alternate_screen = false;
    }
}

void TerminalKernel::write_line(const TerminalLine& line) {
    append_brightness(line.brightness);

    switch (line.type) {
        case TerminalLineType::Error:
            append_output(terminal_constants::ANSI_BOLD);
            append_output(terminal_constants::ANSI_RED);
            append_output(line.prefix, line.prefix_size);
            append_output(line.contents, line.contents_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::Warning:
            append_output(terminal_constants::ANSI_BOLD);
            append_output(terminal_constants::ANSI_YELLOW);
            append_output(line.prefix, line.prefix_size);
            append_output(line.contents, line.contents_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::Info:
            append_output(terminal_constants::ANSI_DIM);
            append_output(line.prefix, line.prefix_size);
            append_output(line.contents, line.contents_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::PreviewLine:
            append_output(terminal_constants::ANSI_DIM);
            append_output(line.prefix, line.prefix_size);
            append_output(line.contents, line.contents_size);
            append_output(line.suffix, line.suffix_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::Header:
            append_output(terminal_constants::ANSI_BOLD);
            append_output(terminal_constants::ANSI_CYAN);
            append_output(line.prefix, line.prefix_size);
            append_output(line.contents, line.contents_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::Separator:
            append_output(terminal_constants::ANSI_DIM);
            append_output(line.contents, line.contents_size);
            append_output(terminal_constants::ANSI_RESET);
            break;
        case TerminalLineType::Filename:
        case TerminalLineType::Dirname:
        case TerminalLineType::Action:
        case TerminalLineType::None:
            append_output(line.contents, line.contents_size);
            break;
    }

    append_output(terminal_constants::ANSI_CLEAR_LINE);
    append_output("\r\n");
    flush_output();
}

void TerminalKernel::write_frame(
    std::span<const TerminalLine> lines,
    std::size_t terminal_cols
) {
    append_output(terminal_constants::ANSI_CURSOR_HOME);

    for (const TerminalLine& line : lines) {
        if (line.type == TerminalLineType::None) {
            append_output(terminal_constants::ANSI_CLEAR_LINE);
            append_output("\r\n");
            continue;
        }

        const bool is_menu_line =
            line.type == TerminalLineType::Filename ||
            line.type == TerminalLineType::Dirname ||
            line.type == TerminalLineType::Action;

        if (is_menu_line) {
            if (line.selected) {
                append_output(terminal_constants::SELECTED_PREFIX);
            } else {
                append_output(terminal_constants::UNSELECTED_PREFIX);
            }
        } else if (line.type != TerminalLineType::PreviewLine) {
            append_brightness(line.brightness);
        }

        switch (line.type) {
            case TerminalLineType::Filename:
                append_menu_brightness(
                    line.brightness,
                    terminal_constants::ANSI_GREEN,
                    terminal_constants::ANSI_DIM_GREEN,
                    terminal_constants::ANSI_DIMMER_GREEN
                );
                append_output(terminal_constants::FILENAME_LABEL);
                append_output(line.contents, line.contents_size);
                break;
            case TerminalLineType::Dirname:
                append_menu_brightness(
                    line.brightness,
                    terminal_constants::ANSI_CYAN,
                    terminal_constants::ANSI_DIM_CYAN,
                    terminal_constants::ANSI_DIMMER_CYAN
                );
                append_output(terminal_constants::DIRNAME_LABEL);
                append_output(line.contents, line.contents_size);
                append_output("/");
                break;
            case TerminalLineType::Action:
                append_menu_brightness(
                    line.brightness,
                    terminal_constants::ANSI_YELLOW,
                    terminal_constants::ANSI_DIM_YELLOW,
                    terminal_constants::ANSI_DIMMER_YELLOW
                );
                append_output(line.prefix, line.prefix_size);
                {
                    const std::size_t prefix_width = line.prefix_size;
                    std::size_t available_width =
                        terminal_cols > prefix_width ? terminal_cols - prefix_width : 0;
                    available_width =
                        available_width > terminal_constants::UNSELECTED_PREFIX.size()
                            ? available_width - terminal_constants::UNSELECTED_PREFIX.size()
                            : 0;
                    append_middle_truncated(
                        std::string_view(line.contents, line.contents_size),
                        available_width
                    );
                }
                break;
            case TerminalLineType::Error:
                append_output(terminal_constants::ANSI_BOLD);
                append_output(terminal_constants::ANSI_RED);
                append_output(line.prefix, line.prefix_size);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::Warning:
                append_output(terminal_constants::ANSI_BOLD);
                append_output(terminal_constants::ANSI_YELLOW);
                append_output(line.prefix, line.prefix_size);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::Info:
                append_output(terminal_constants::ANSI_DIM);
                append_output(line.prefix, line.prefix_size);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::PreviewLine:
                append_output(line.prefix, line.prefix_size);
                append_brightness(line.brightness);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::Header:
                append_output(terminal_constants::ANSI_BOLD);
                append_output(terminal_constants::ANSI_CYAN);
                append_output(line.prefix, line.prefix_size);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::Separator:
                append_output(terminal_constants::ANSI_DIM);
                append_output(line.contents, line.contents_size);
                append_output(line.suffix, line.suffix_size);
                break;
            case TerminalLineType::None:
                break;
        }

        append_output(terminal_constants::ANSI_RESET);
        append_output(terminal_constants::ANSI_CLEAR_LINE);
        append_output("\r\n");
    }

    append_output(terminal_constants::ANSI_CLEAR_FROM_CURSOR);
    flush_output();
}

} // namespace arex::kernels
