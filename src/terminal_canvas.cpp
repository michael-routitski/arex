#include <arex/terminal_canvas.hpp>

#include <algorithm>
#include <cstring>
#include <span>

namespace arex {

TerminalCanvas::TerminalCanvas()
    : _terminal_lines{} {
    const TerminalSize term_size = kernels::TerminalKernel::get_terminal_size();
    _current_terminal_lines = std::min(
        term_size.rows,
        _terminal_lines.size()
    );
}

TerminalCanvas::~TerminalCanvas() {
    leave_alternate_screen();
}

void TerminalCanvas::enter_alternate_screen() {
    _terminal_kernel.enter_alternate_screen();
}

void TerminalCanvas::leave_alternate_screen() {
    _terminal_kernel.leave_alternate_screen();
}

TerminalSize TerminalCanvas::get_terminal_size() {
    return kernels::TerminalKernel::get_terminal_size();
}

void TerminalCanvas::set_line(
    std::size_t line_index,
    std::string_view contents,
    std::string_view prefix,
    std::string_view suffix,
    TerminalLineType type,
    bool selected,
    TerminalBrightness brightness
) noexcept {
    if (line_index >= _current_terminal_lines) {
        return;
    }

    TerminalLine& line = _terminal_lines[line_index];
    line.contents = contents.data();
    line.contents_size = contents.size();
    line.prefix = prefix.data();
    line.prefix_size = prefix.size();
    line.suffix = suffix.data();
    line.suffix_size = suffix.size();
    line.type = type;
    line.brightness = brightness;
    line.selected = selected;
}

void TerminalCanvas::set_filename(
    std::size_t line_index,
    std::string_view filename,
    bool selected,
    TerminalBrightness brightness
) noexcept {
    set_line(
        line_index,
        filename,
        {},
        {},
        TerminalLineType::Filename,
        selected,
        brightness
    );
}

void TerminalCanvas::set_dirname(
    std::size_t line_index,
    std::string_view dirname,
    bool selected,
    TerminalBrightness brightness
) noexcept {
    set_line(
        line_index,
        dirname,
        {},
        {},
        TerminalLineType::Dirname,
        selected,
        brightness
    );
}

void TerminalCanvas::set_action(
    std::size_t line_index,
    std::string_view action,
    bool selected,
    TerminalBrightness brightness,
    std::string_view action_label
) noexcept {
    set_line(
        line_index,
        action,
        action_label,
        {},
        TerminalLineType::Action,
        selected,
        brightness
    );
}

void TerminalCanvas::set_error(
    std::size_t line_index,
    std::string_view error
) noexcept {
    set_line(
        line_index,
        error,
        terminal_constants::ERROR_LABEL,
        {},
        TerminalLineType::Error,
        false,
        TerminalBrightness::Default
    );
}

void TerminalCanvas::set_warning(
    std::size_t line_index,
    std::string_view warning
) noexcept {
    set_line(
        line_index,
        warning,
        terminal_constants::WARNING_LABEL,
        {},
        TerminalLineType::Warning,
        false,
        TerminalBrightness::Default
    );
}

void TerminalCanvas::set_info(
    std::size_t line_index,
    std::string_view info,
    bool selected,
    TerminalBrightness brightness
) noexcept {
    set_line(
        line_index,
        info,
        terminal_constants::INFO_PREFIX,
        {},
        TerminalLineType::Info,
        selected,
        brightness
    );
}

void TerminalCanvas::set_info(
    std::size_t line_index,
    std::string_view info,
    std::string_view prefix,
    std::string_view suffix,
    bool selected,
    TerminalBrightness brightness
) noexcept {
    set_line(
        line_index,
        info,
        prefix,
        suffix,
        TerminalLineType::Info,
        selected,
        brightness
    );
}

void TerminalCanvas::set_preview_line(
    std::size_t line_index,
    std::string_view preview_line,
    TerminalBrightness brightness
) noexcept {
    set_line(
        line_index,
        preview_line,
        terminal_constants::PREVIEW_PREFIX,
        {},
        TerminalLineType::PreviewLine,
        false,
        brightness
    );
}

void TerminalCanvas::set_header(
    std::size_t line_index,
    std::string_view header
) noexcept {
    set_line(
        line_index,
        header,
        terminal_constants::HEADER_LABEL,
        {},
        TerminalLineType::Header,
        false,
        TerminalBrightness::Default
    );
}

void TerminalCanvas::set_separator(
    std::size_t line_index,
    std::string_view separator
) noexcept {
    set_line(
        line_index,
        separator,
        {},
        {},
        TerminalLineType::Separator,
        false,
        TerminalBrightness::Default
    );
}

void TerminalCanvas::reset_terminal_lines() {
    _terminal_line_count = 0;
    std::memset(
        _terminal_lines.data(),
        0,
        _terminal_lines.size() * sizeof(TerminalLine)
    );
}

void TerminalCanvas::render(const RenderState& state) {
    const TerminalSize term_size = kernels::TerminalKernel::get_terminal_size();
    _current_terminal_lines = std::min(
        term_size.rows,
        _terminal_lines.size()
    );
    reset_terminal_lines();

    constexpr std::size_t fixed_header_lines =
        1 + 1 + 1 + 1;
    const std::size_t available_content_lines =
        (_current_terminal_lines > fixed_header_lines)
            ? _current_terminal_lines - fixed_header_lines
            : 0;
    const std::size_t preview_height =
        available_content_lines / 3;
    const std::size_t available_menu_rows =
        available_content_lines - preview_height;

    std::size_t line_index = 0;
    set_header(line_index, state.current_path);
    ++line_index;
    set_separator(line_index, terminal_constants::PREVIEW_TOP_SEPARATOR);
    ++line_index;

    const bool preview_overflow =
        state.preview_overflow ||
        state.preview_lines.size() > preview_height;

    for (std::size_t i = 0; i < preview_height; ++i) {
        TerminalBrightness preview_brightness =
            TerminalBrightness::Default;
        if (preview_overflow) {
            if (i + 2 == preview_height) {
                preview_brightness = TerminalBrightness::Dim;
            } else if (i + 1 == preview_height) {
                preview_brightness = TerminalBrightness::Dimmer;
            }
        }

        if (i < state.preview_lines.size()) {
            set_preview_line(
                line_index,
                state.preview_lines[i],
                preview_brightness
            );
        } else {
            set_preview_line(line_index, "", preview_brightness);
        }
        ++line_index;
    }

    set_separator(line_index, terminal_constants::PREVIEW_BOTTOM_SEPARATOR);
    ++line_index;

    const std::size_t total_items = state.menu_items.size();
    std::size_t start_idx = 0;
    std::size_t end_idx = total_items;

    if (total_items > available_menu_rows) {
        constexpr std::size_t edge_distance = 3;
        constexpr std::size_t dimmed_boundary_rows = 2;
        start_idx = std::min(
            _viewport_start,
            total_items - available_menu_rows
        );

        if (state.selected_index <= edge_distance - 1) {
            start_idx = 0;
        } else if (state.selected_index >= total_items - edge_distance) {
            start_idx = total_items - available_menu_rows;
        } else if (start_idx > 0 &&
                   state.selected_index <= start_idx + edge_distance - 1) {
            start_idx = start_idx - 1;
        } else if (start_idx + available_menu_rows <
                       total_items &&
                   state.selected_index >=
                       start_idx + available_menu_rows - edge_distance) {
            start_idx = std::min(
                total_items - available_menu_rows,
                state.selected_index -
                    (available_menu_rows - edge_distance) +
                    dimmed_boundary_rows - 1
            );
        }
        end_idx = std::min(total_items, start_idx + available_menu_rows);
    } else {
        start_idx = 0;
        end_idx = total_items;
    }

    _viewport_start = start_idx;

    for (std::size_t i = start_idx; i < end_idx; ++i) {
        const MenuItem& item = state.menu_items[i];
        const bool is_selected = (i == state.selected_index);
        TerminalBrightness brightness = TerminalBrightness::Default;

        if (is_selected) {
            brightness = TerminalBrightness::Highlight;
        } else {
            if (start_idx > 0 && i == start_idx) {
                brightness = TerminalBrightness::Dimmer;
            } else if (start_idx > 0 && i == start_idx + 1) {
                brightness = TerminalBrightness::Dim;
            }

            if (end_idx < total_items && i + 1 == end_idx) {
                brightness = TerminalBrightness::Dimmer;
            } else if (end_idx < total_items && i + 2 == end_idx) {
                brightness = TerminalBrightness::Dim;
            }
        }

        switch (item.kind) {
            case ItemKind::Action:
                set_action(
                    line_index,
                    item.name,
                    is_selected,
                    brightness,
                    item.action_label.empty()
                        ? terminal_constants::ACTION_LABEL
                        : item.action_label
                );
                break;
            case ItemKind::Directory:
                set_dirname(line_index, item.name, is_selected, brightness);
                break;
            case ItemKind::File:
            default:
                set_filename(line_index, item.name, is_selected, brightness);
                break;
        }
        ++line_index;
    }

    if (!state.status_message.empty()) {
        set_info(line_index, state.status_message);
        ++line_index;
    }

    _terminal_line_count = std::min(
        line_index,
        _current_terminal_lines
    );
    _terminal_kernel.write_frame(
        std::span<const TerminalLine>(
            _terminal_lines.data(),
            _terminal_line_count
        ),
        term_size.cols
    );
}

} // namespace arex
