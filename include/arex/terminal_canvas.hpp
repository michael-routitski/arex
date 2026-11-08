#ifndef AREX_TERMINAL_CANVAS_HPP
#define AREX_TERMINAL_CANVAS_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace arex {

enum class ItemKind : std::uint8_t {
    File = 0,
    Directory,
    Action
};

struct MenuItem {
    std::string name;
    ItemKind kind{ItemKind::File};
    std::uintmax_t file_size{0};
};

struct RenderState {
    std::vector<std::string> preview_lines;
    std::vector<MenuItem> menu_items;
    std::size_t selected_index{0};
    std::string current_path;
    std::string status_message;
    std::size_t preview_height{8};
};

class TerminalCanvas {
public:
    explicit TerminalCanvas(std::size_t initial_buffer_capacity = 8192);
    ~TerminalCanvas();

    TerminalCanvas(const TerminalCanvas&) = delete;
    TerminalCanvas& operator=(const TerminalCanvas&) = delete;

    TerminalCanvas(TerminalCanvas&&) noexcept = default;
    TerminalCanvas& operator=(TerminalCanvas&&) noexcept = default;

    void enter_alternate_screen();
    void leave_alternate_screen();
    void render(const RenderState& state);

private:
    void append_ansi(std::string_view code);
    void clear_line_to_end();
    void write_raw_buffer();

    std::string m_buffer;
    bool m_in_alternate_screen{false};
};

} // namespace arex

#endif // AREX_TERMINAL_CANVAS_HPP