#ifndef AREX_MENU_NAVIGATOR_HPP
#define AREX_MENU_NAVIGATOR_HPP

#include <arex/directory_scanner.hpp>
#include <arex/file_previewer.hpp>
#include <arex/kernels/keyboard_kernel.hpp>
#include <arex/terminal_canvas.hpp>

#include <filesystem>
#include <vector>

namespace arex {

enum class MenuEventType : std::uint8_t {
    Handled = 0,
    Exit,
    OpenFileActions,
    OpenDirectoryActions,
    CloseActions
};

struct MenuEvent {
    MenuEventType type{MenuEventType::Handled};
    std::filesystem::path target{};
};

class MenuNavigator {
public:
    MenuNavigator();
    ~MenuNavigator() = default;

    MenuNavigator(const MenuNavigator&) = delete;
    MenuNavigator& operator=(const MenuNavigator&) = delete;
    MenuNavigator(MenuNavigator&&) noexcept = default;
    MenuNavigator& operator=(MenuNavigator&&) noexcept = default;

    [[nodiscard]] bool initialize(int argc, char** argv);
    void render();
    MenuEvent handle_key(kernels::KeyCode key);
    void refresh();
    void leave_alternate_screen();

    [[nodiscard]] bool running() const noexcept;
    [[nodiscard]] TerminalCanvas& canvas() noexcept;
    [[nodiscard]] kernels::KeyboardKernel& keyboard_kernel() noexcept;
    [[nodiscard]] DirectoryScanner& scanner() noexcept;
    [[nodiscard]] std::filesystem::path& current_dir() noexcept;
    [[nodiscard]] const std::filesystem::path& current_dir() const noexcept;
    [[nodiscard]] std::vector<DirectoryItem>& scanned_items() noexcept;
    [[nodiscard]] RenderState& render_state() noexcept;
    [[nodiscard]] std::size_t& selected_index() noexcept;

private:
    void update_render_menu();
    void update_preview();

    TerminalCanvas _canvas;
    kernels::KeyboardKernel _keyboard_kernel;
    DirectoryScanner _scanner;
    FilePreviewer _previewer;
    std::vector<DirectoryItem> _scanned_items;
    std::vector<char> _preview_buffer;
    RenderState _render_state;
    std::filesystem::path _current_dir;
    std::filesystem::path _initial_root;
    std::size_t _selected_index{0};
    bool _running{true};
};

} // namespace arex

#endif // AREX_MENU_NAVIGATOR_HPP
