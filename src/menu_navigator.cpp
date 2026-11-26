#include <arex/menu_navigator.hpp>

#include <algorithm>
#include <filesystem>

namespace arex {

MenuNavigator::MenuNavigator()
    : _previewer(65, 16384) {}

bool MenuNavigator::initialize(int argc, char** argv) {
    _current_dir = std::filesystem::current_path();
    if (argc > 1) {
        std::filesystem::path requested_path(argv[1]);
        std::error_code ec;
        if (std::filesystem::exists(requested_path, ec) &&
            std::filesystem::is_directory(requested_path, ec)) {
            _current_dir = std::filesystem::canonical(requested_path, ec);
        }
    }
    _initial_root = _current_dir;

    if (!_scanner.scan(_current_dir, _scanned_items)) {
        return false;
    }
    refresh();
    return true;
}

void MenuNavigator::render() {
    _canvas.render(_render_state);
}

void MenuNavigator::leave_alternate_screen() {
    _canvas.leave_alternate_screen();
}

bool MenuNavigator::running() const noexcept {
    return _running;
}

TerminalCanvas& MenuNavigator::canvas() noexcept {
    return _canvas;
}

kernels::KeyboardKernel& MenuNavigator::keyboard_kernel() noexcept {
    return _keyboard_kernel;
}

DirectoryScanner& MenuNavigator::scanner() noexcept {
    return _scanner;
}

std::filesystem::path& MenuNavigator::current_dir() noexcept {
    return _current_dir;
}

const std::filesystem::path& MenuNavigator::current_dir() const noexcept {
    return _current_dir;
}

std::vector<DirectoryItem>& MenuNavigator::scanned_items() noexcept {
    return _scanned_items;
}

RenderState& MenuNavigator::render_state() noexcept {
    return _render_state;
}

std::size_t& MenuNavigator::selected_index() noexcept {
    return _selected_index;
}

void MenuNavigator::update_render_menu() {
    _render_state.menu_items.clear();
    _render_state.menu_items.reserve(_scanned_items.size() + 1);

    for (const auto& item : _scanned_items) {
        _render_state.menu_items.push_back(MenuItem{
            .name = item.name,
            .kind = item.type == FileType::Directory
                ? ItemKind::Directory
                : ItemKind::File,
            .file_size = 0
        });
    }

    _render_state.menu_items.push_back(MenuItem{
        .name = "Dir Actions ...",
        .kind = ItemKind::Action,
        .file_size = 0,
        .action_label = kernels::terminal_constants::ROOT_ACTION_LABEL
    });
}

void MenuNavigator::update_preview() {
    _render_state.preview_lines.clear();
    _render_state.preview_overflow = false;
    constexpr std::size_t max_lines = 64;

    if (_scanned_items.empty() || _selected_index >= _scanned_items.size()) {
        if (_selected_index == _scanned_items.size()) {
            _render_state.preview_lines.emplace_back(
                "[ Folder Actions: Perform operations on current directory ]"
            );
        } else {
            _render_state.preview_lines.emplace_back("[ Empty Directory ]");
        }
        return;
    }

    const DirectoryItem& selected_item = _scanned_items[_selected_index];
    const std::filesystem::path target_path =
        _current_dir / selected_item.name;

    if (selected_item.type == FileType::Directory) {
        std::vector<DirectoryItem> dir_contents;
        if (_scanner.scan(target_path, dir_contents) && !dir_contents.empty()) {
            const std::size_t limit =
                max_lines > 1 ? max_lines - 1 : 1;
            const std::size_t count =
                std::min(limit, dir_contents.size());

            for (std::size_t i = 0; i < count; ++i) {
                const std::string prefix =
                    dir_contents[i].type == FileType::Directory
                        ? "[DIR]  "
                        : "       ";
                _render_state.preview_lines.push_back(
                    prefix + dir_contents[i].name
                );
            }

            if (dir_contents.size() > limit) {
                _render_state.preview_lines.emplace_back("...");
                _render_state.preview_overflow = true;
            }
        } else {
            _render_state.preview_lines.emplace_back("[ Empty Directory ]");
        }
        return;
    }

    PreviewResult result;
    if (_previewer.preview_file(target_path, _preview_buffer, result)) {
        if (result.is_binary) {
            _render_state.preview_lines.emplace_back("[ Binary File ]");
        } else if (result.is_empty) {
            _render_state.preview_lines.emplace_back("[ Empty File ]");
        } else {
            _render_state.preview_lines.reserve(result.lines.size());
            for (const auto line_view : result.lines) {
                _render_state.preview_lines.emplace_back(line_view);
            }
            _render_state.preview_overflow = result.has_overflow;
        }
    } else {
        _render_state.preview_lines.emplace_back(
            "[ Error reading preview ]"
        );
    }
}

void MenuNavigator::refresh() {
    _render_state.current_path = _current_dir.string();
    update_render_menu();
    update_preview();
    if (_render_state.status_message.empty()) {
        _render_state.status_message =
            "j/k: move | Enter/l: open/actions | h/Backspace: back "
            "(Exit at launch dir) | q: quit";
    }
}

MenuEvent MenuNavigator::handle_key(kernels::KeyCode key) {
    const std::size_t total_menu_count = _scanned_items.size() + 1;

    switch (key) {
        case kernels::KeyCode::Up:
            if (_selected_index > 0) {
                --_selected_index;
                _render_state.selected_index = _selected_index;
                _render_state.status_message.clear();
                update_preview();
            }
            break;
        case kernels::KeyCode::Down:
            if (_selected_index + 1 < total_menu_count) {
                ++_selected_index;
                _render_state.selected_index = _selected_index;
                _render_state.status_message.clear();
                update_preview();
            }
            break;
        case kernels::KeyCode::Home:
            _selected_index = 0;
            _render_state.selected_index = 0;
            _render_state.status_message.clear();
            update_preview();
            break;
        case kernels::KeyCode::End:
            _selected_index = total_menu_count - 1;
            _render_state.selected_index = _selected_index;
            _render_state.status_message.clear();
            update_preview();
            break;
        case kernels::KeyCode::Right:
        case kernels::KeyCode::Enter:
            if (_selected_index == _scanned_items.size()) {
                return {
                    MenuEventType::OpenDirectoryActions,
                    _current_dir
                };
            }
            if (_selected_index < _scanned_items.size()) {
                const DirectoryItem& selected =
                    _scanned_items[_selected_index];
                const std::filesystem::path target =
                    _current_dir / selected.name;
                if (selected.type == FileType::Directory) {
                    std::error_code ec;
                    if (_scanner.scan(target, _scanned_items)) {
                        _current_dir =
                            std::filesystem::canonical(target, ec);
                        _selected_index = 0;
                        _render_state.selected_index = 0;
                        _render_state.status_message.clear();
                        refresh();
                    }
                } else {
                    return {
                        MenuEventType::OpenFileActions,
                        target
                    };
                }
            }
            break;
        case kernels::KeyCode::Left:
        case kernels::KeyCode::Backspace:
            if (_current_dir == _initial_root) {
                _running = false;
            } else if (_current_dir.has_parent_path()) {
                const std::filesystem::path parent =
                    _current_dir.parent_path();
                if (_scanner.scan(parent, _scanned_items)) {
                    _current_dir = parent;
                    _selected_index = 0;
                    _render_state.selected_index = 0;
                    _render_state.status_message.clear();
                    refresh();
                }
            }
            break;
        case kernels::KeyCode::Escape:
            _running = false;
            break;
        default:
            break;
    }

    return MenuEvent{};
}

} // namespace arex
