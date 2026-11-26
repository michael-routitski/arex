#ifndef AREX_KERNELS_TERMINAL_KERNEL_HPP
#define AREX_KERNELS_TERMINAL_KERNEL_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace arex::kernels {

enum class TerminalLineType : std::uint8_t {
    None = 0,
    Filename,
    Dirname,
    Action,
    Error,
    Warning,
    Info,
    PreviewLine,
    Header,
    Separator
};

enum class TerminalBrightness : std::uint8_t {
    Highlight = 0,
    Default,
    Dim,
    Dimmer
};

struct TerminalLine {
    const char* contents;
    std::size_t contents_size;
    const char* prefix;
    std::size_t prefix_size;
    const char* suffix;
    std::size_t suffix_size;
    TerminalLineType type;
    TerminalBrightness brightness;
    bool selected;
};

struct TerminalSize {
    std::size_t rows{24};
    std::size_t cols{80};
};

namespace terminal_constants {
inline constexpr std::string_view ANSI_ENTER_ALT_SCREEN = "\x1b[?1049h";
inline constexpr std::string_view ANSI_LEAVE_ALT_SCREEN = "\x1b[?1049l";
inline constexpr std::string_view ANSI_CURSOR_HOME = "\x1b[H";
inline constexpr std::string_view ANSI_CLEAR_FROM_CURSOR = "\x1b[J";
inline constexpr std::string_view ANSI_HIDE_CURSOR = "\x1b[?25l";
inline constexpr std::string_view ANSI_SHOW_CURSOR = "\x1b[?25h";
inline constexpr std::string_view ANSI_CLEAR_LINE = "\x1b[K";
inline constexpr std::string_view ANSI_RESET = "\x1b[0m";
inline constexpr std::string_view ANSI_BOLD = "\x1b[1m";
inline constexpr std::string_view ANSI_DIM = "\x1b[2m";
inline constexpr std::string_view ANSI_CYAN = "\x1b[36m";
inline constexpr std::string_view ANSI_GREEN = "\x1b[32m";
inline constexpr std::string_view ANSI_RED = "\x1b[31m";
inline constexpr std::string_view ANSI_YELLOW = "\x1b[33m";
inline constexpr std::string_view ANSI_DIM_TEXT = "\x1b[38;2;160;160;160m";
inline constexpr std::string_view ANSI_DIMMER_TEXT = "\x1b[38;2;96;96;96m";
inline constexpr std::string_view ANSI_DIM_GREEN = "\x1b[38;2;0;128;64m";
inline constexpr std::string_view ANSI_DIMMER_GREEN = "\x1b[38;2;0;72;36m";
inline constexpr std::string_view ANSI_DIM_CYAN = "\x1b[38;2;0;128;128m";
inline constexpr std::string_view ANSI_DIMMER_CYAN = "\x1b[38;2;0;72;72m";
inline constexpr std::string_view ANSI_DIM_YELLOW = "\x1b[38;2;160;160;0m";
inline constexpr std::string_view ANSI_DIMMER_YELLOW = "\x1b[38;2;96;96;0m";
inline constexpr std::string_view ANSI_INVERSE = "\x1b[7m";

inline constexpr std::string_view FILENAME_LABEL = "[FILE] ";
inline constexpr std::string_view DIRNAME_LABEL = "[DIR] ";
inline constexpr std::string_view ACTION_LABEL = "[ACTION] ";
inline constexpr std::string_view ROOT_ACTION_LABEL = "[> ...] ";
inline constexpr std::string_view ERROR_LABEL = "[ERROR] ";
inline constexpr std::string_view WARNING_LABEL = "[WARNING] ";
inline constexpr std::string_view HEADER_LABEL = " [arex] ";
inline constexpr std::string_view PREVIEW_PREFIX = "│ ";
inline constexpr std::string_view INFO_PREFIX = " -- ";
inline constexpr std::string_view SELECTED_PREFIX = " > ";
inline constexpr std::string_view UNSELECTED_PREFIX = "   ";
inline constexpr std::string_view MENU_BOUNDARY_PREFIX = "... [ ";
inline constexpr std::string_view MENU_BOUNDARY_SUFFIX = " ]";
inline constexpr std::string_view PREVIEW_TOP_SEPARATOR = "┌─ Preview ──────────────────────────────────────────";
inline constexpr std::string_view PREVIEW_BOTTOM_SEPARATOR = "└────────────────────────────────────────────────────";
} // namespace terminal_constants

class TerminalKernel {
public:
    TerminalKernel() = default;
    ~TerminalKernel() = default;

    TerminalKernel(const TerminalKernel&) = delete;
    TerminalKernel& operator=(const TerminalKernel&) = delete;
    TerminalKernel(TerminalKernel&&) noexcept = default;
    TerminalKernel& operator=(TerminalKernel&&) noexcept = default;

    [[nodiscard]] static TerminalSize get_terminal_size();

    void enter_alternate_screen();
    void leave_alternate_screen();
    void write_frame(std::span<const TerminalLine> lines, std::size_t terminal_cols);
    void write_line(const TerminalLine& line);

private:
    void append_output(std::string_view contents);
    void append_output(const char* contents, std::size_t size);
    void append_middle_truncated(std::string_view contents, std::size_t maximum_width);
    void append_brightness(TerminalBrightness brightness);
    void append_menu_brightness(
        TerminalBrightness brightness,
        std::string_view normal,
        std::string_view dim,
        std::string_view dimmer
    );
    void flush_output();

    std::array<char, 8192> _output_buffer{};
    std::size_t _output_size{0};
    bool _in_alternate_screen{false};
};

} // namespace arex::kernels

#endif // AREX_KERNELS_TERMINAL_KERNEL_HPP
