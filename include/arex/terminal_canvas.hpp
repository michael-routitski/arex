#ifndef AREX_TERMINAL_CANVAS_HPP
#define AREX_TERMINAL_CANVAS_HPP

#include <arex/kernels/terminal_kernel.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace arex {

using TerminalLineType = kernels::TerminalLineType;
using TerminalBrightness = kernels::TerminalBrightness;
using TerminalLine = kernels::TerminalLine;
using TerminalSize = kernels::TerminalSize;
namespace terminal_constants = kernels::terminal_constants;

enum class ItemKind : std::uint8_t {
    File = 0,
    Directory,
    Action
};

struct MenuItem {
    std::string name;
    ItemKind kind{ItemKind::File};
    std::uintmax_t file_size{0};
    std::string_view action_label{};
};

struct RenderState {
    std::vector<std::string> preview_lines;
    bool preview_overflow{false};
    std::vector<MenuItem> menu_items;
    std::size_t selected_index{0};
    std::string current_path;
    std::string status_message;
};

class TerminalCanvas {
public:
    TerminalCanvas();
    ~TerminalCanvas();

    TerminalCanvas(const TerminalCanvas&) = delete;
    TerminalCanvas& operator=(const TerminalCanvas&) = delete;

    TerminalCanvas(TerminalCanvas&&) noexcept = default;
    TerminalCanvas& operator=(TerminalCanvas&&) noexcept = default;

    void enter_alternate_screen();
    void leave_alternate_screen();

    /// Queries terminal dimensions through TerminalKernel.
    [[nodiscard]] static TerminalSize get_terminal_size();

    /// Renders state with automatic menu scrolling bounded to current terminal height.
    void render(const RenderState& state);

private:
    void set_filename(
        std::size_t line_index,
        std::string_view filename,
        bool selected = false,
        TerminalBrightness brightness = TerminalBrightness::Default
    ) noexcept;
    void set_dirname(
        std::size_t line_index,
        std::string_view dirname,
        bool selected = false,
        TerminalBrightness brightness = TerminalBrightness::Default
    ) noexcept;
    void set_action(
        std::size_t line_index,
        std::string_view action,
        bool selected = false,
        TerminalBrightness brightness = TerminalBrightness::Default,
        std::string_view action_label = terminal_constants::ACTION_LABEL
    ) noexcept;
    void set_error(std::size_t line_index, std::string_view error) noexcept;
    void set_warning(std::size_t line_index, std::string_view warning) noexcept;
    void set_info(
        std::size_t line_index,
        std::string_view info,
        bool selected = false,
        TerminalBrightness brightness = TerminalBrightness::Default
    ) noexcept;
    void set_info(
        std::size_t line_index,
        std::string_view info,
        std::string_view prefix,
        std::string_view suffix,
        bool selected,
        TerminalBrightness brightness = TerminalBrightness::Default
    ) noexcept;
    void set_preview_line(
        std::size_t line_index,
        std::string_view preview_line,
        TerminalBrightness brightness = TerminalBrightness::Default
    ) noexcept;
    void set_header(std::size_t line_index, std::string_view header) noexcept;
    void set_separator(std::size_t line_index, std::string_view separator) noexcept;
    void set_line(
        std::size_t line_index,
        std::string_view contents,
        std::string_view prefix,
        std::string_view suffix,
        TerminalLineType type,
        bool selected,
        TerminalBrightness brightness
    ) noexcept;
    void reset_terminal_lines();

    std::array<TerminalLine, 64> _terminal_lines{};
    std::size_t _current_terminal_lines{0};
    std::size_t _terminal_line_count{0};
    std::size_t _viewport_start{0};
    kernels::TerminalKernel _terminal_kernel;
};

} // namespace arex

#endif // AREX_TERMINAL_CANVAS_HPP