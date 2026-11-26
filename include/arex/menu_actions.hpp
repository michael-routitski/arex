#ifndef AREX_MENU_ACTIONS_HPP
#define AREX_MENU_ACTIONS_HPP

#include <arex/directory_scanner.hpp>
#include <arex/action.hpp>
#include <arex/kernels/keyboard_kernel.hpp>
#include <arex/menu_navigator.hpp>
#include <arex/terminal_canvas.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace arex {

class MenuActions {
public:
    MenuActions(
        TerminalCanvas& canvas,
        kernels::KeyboardKernel& keyboard_kernel,
        DirectoryScanner& scanner,
        std::filesystem::path& current_dir,
        std::vector<DirectoryItem>& scanned_items,
        std::size_t& navigation_selected_index,
        RenderState& render_state
    );

    void open_file(const std::filesystem::path& target);
    void open_directory(const std::filesystem::path& target);
    void close(bool clear_status = true);
    MenuEvent handle_key(kernels::KeyCode key);

private:
    void rebuild_render_menu();
    ActionContext make_action_context();

    TerminalCanvas& _canvas;
    kernels::KeyboardKernel& _keyboard_kernel;
    DirectoryScanner& _scanner;
    std::filesystem::path& _current_dir;
    std::vector<DirectoryItem>& _scanned_items;
    std::size_t& _navigation_selected_index;
    RenderState& _render_state;
    std::vector<ActionItem> _actions;
    std::filesystem::path _target;
    std::size_t _selected_index{0};
    ActionClipboard _clipboard{};
};

} // namespace arex

#endif // AREX_MENU_ACTIONS_HPP
