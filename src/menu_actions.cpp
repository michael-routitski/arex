#include <arex/menu_actions.hpp>

#include "actions/cat.hpp"
#include "actions/copy.hpp"
#include "actions/copy_full.hpp"
#include "actions/copy_relative.hpp"
#include "actions/cut.hpp"
#include "actions/default.hpp"
#include "actions/less.hpp"
#include "actions/ls.hpp"
#include "actions/more.hpp"
#include "actions/paste.hpp"
#include "actions/tree.hpp"
#include "actions/tree_directory.hpp"

#include <system_error>

namespace arex {

MenuActions::MenuActions(
    TerminalCanvas& canvas,
    kernels::KeyboardKernel& keyboard_kernel,
    DirectoryScanner& scanner,
    std::filesystem::path& current_dir,
    std::vector<DirectoryItem>& scanned_items,
    std::size_t& navigation_selected_index,
    RenderState& render_state
)
    : _canvas(canvas),
      _keyboard_kernel(keyboard_kernel),
      _scanner(scanner),
      _current_dir(current_dir),
      _scanned_items(scanned_items),
      _navigation_selected_index(navigation_selected_index),
      _render_state(render_state) {}

void MenuActions::rebuild_render_menu() {
    _render_state.menu_items.clear();
    _render_state.menu_items.reserve(_actions.size());
    for (const ActionItem& action : _actions) {
        _render_state.menu_items.push_back(MenuItem{
            .name = action.label,
            .kind = ItemKind::Action,
            .file_size = 0
        });
    }
}

ActionContext MenuActions::make_action_context() {
    return ActionContext{
        .target = _target,
        .current_dir = _current_dir,
        .clipboard = _clipboard,
        .canvas = _canvas,
        .keyboard_kernel = _keyboard_kernel,
        .scanner = _scanner,
        .scanned_items = _scanned_items,
        .render_state = _render_state
    };
}

void MenuActions::open_file(const std::filesystem::path& target) {
    _target = target;
    _actions.clear();
    _actions.push_back(make_cat_action());

    std::error_code ec;
    const std::filesystem::path relative =
        std::filesystem::relative(target, _current_dir, ec);
    const std::string relative_value =
        ec ? target.filename().string() : relative.string();
    const std::string full_value = target.lexically_normal().string();

    _actions.push_back({
        "Copy Path `" + relative_value + "`",
        "",
        {},
        ActionExecution::Builtin,
        copy_relative_action
    });
    _actions.push_back({
        "Copy Path `" + full_value + "`",
        "",
        {},
        ActionExecution::Builtin,
        copy_full_action
    });
    _actions.push_back(make_default_action());
    _actions.push_back(make_more_action());
    _actions.push_back(make_less_action());
    _actions.push_back({
        "cut",
        "",
        {},
        ActionExecution::Builtin,
        cut_action
    });
    _actions.push_back({
        "copy",
        "",
        {},
        ActionExecution::Builtin,
        copy_action
    });
    if (_clipboard.has_content) {
        _actions.push_back({
            "paste (" + _clipboard.source_path.filename().string() + ")",
            "",
            {},
            ActionExecution::Builtin,
            paste_action
        });
    }

    _selected_index = 0;
    rebuild_render_menu();
    _render_state.selected_index = 0;
    _render_state.status_message =
        "[File Actions] Target: " + target.filename().string() +
        " (Esc to cancel)";
}

void MenuActions::open_directory(const std::filesystem::path& target) {
    _target = target;
    _actions.clear();

    std::error_code ec;
    const std::filesystem::path relative =
        std::filesystem::relative(target, target.parent_path(), ec);
    const std::string relative_value =
        ec ? target.filename().string() : relative.string();
    const std::string full_value = target.lexically_normal().string();

    _actions.push_back({
        "Copy Path `" + relative_value + "`",
        "",
        {},
        ActionExecution::Builtin,
        copy_relative_action
    });
    _actions.push_back({
        "Copy Path `" + full_value + "`",
        "",
        {},
        ActionExecution::Builtin,
        copy_full_action
    });
    _actions.push_back({
        "cut",
        "",
        {},
        ActionExecution::Builtin,
        cut_action
    });
    _actions.push_back({
        "copy",
        "",
        {},
        ActionExecution::Builtin,
        copy_action
    });
    if (_clipboard.has_content) {
        _actions.push_back({
            "paste (" + _clipboard.source_path.filename().string() + ")",
            "",
            {},
            ActionExecution::Builtin,
            paste_action
        });
    }
    _actions.push_back(make_ls_action());
    _actions.push_back(make_tree_action());
    _actions.push_back(make_tree_directory_action());

    _selected_index = 0;
    rebuild_render_menu();
    _render_state.selected_index = 0;
    _render_state.status_message =
        "[Folder Actions] Target: " + target.filename().string() +
        " (Esc to cancel)";
}

void MenuActions::close(bool clear_status) {
    _render_state.selected_index = _navigation_selected_index;
    if (clear_status) {
        _render_state.status_message.clear();
    }
    _actions.clear();
}

MenuEvent MenuActions::handle_key(kernels::KeyCode key) {
    switch (key) {
        case kernels::KeyCode::Up:
            if (_selected_index > 0) {
                --_selected_index;
                _render_state.selected_index = _selected_index;
            }
            break;
        case kernels::KeyCode::Down:
            if (_selected_index + 1 < _actions.size()) {
                ++_selected_index;
                _render_state.selected_index = _selected_index;
            }
            break;
        case kernels::KeyCode::Enter:
        case kernels::KeyCode::Right: {
            if (_actions.empty()) {
                break;
            }

            const ActionItem& chosen = _actions[_selected_index];
            if (chosen.execute != nullptr) {
                ActionContext context = make_action_context();
                chosen.execute(context, chosen);
            }

            close(false);
            return {MenuEventType::CloseActions, _target};
        }
        case kernels::KeyCode::Escape:
        case kernels::KeyCode::Left:
        case kernels::KeyCode::Backspace:
            close();
            return {MenuEventType::CloseActions, _target};
        default:
            break;
    }
    return MenuEvent{};
}

} // namespace arex
