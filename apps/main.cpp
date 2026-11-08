#include <arex/directory_scanner.hpp>
#include <arex/file_previewer.hpp>
#include <arex/key_listener.hpp>
#include <arex/process_runner.hpp>
#include <arex/terminal_canvas.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

enum class Mode {
    DirectoryNavigation,
    FileActions,
    DirectoryActions
};

struct ActionItem {
    std::string label;
    std::string command;
    std::vector<std::string> args;
    bool is_builtin{false};
    enum class BuiltinType { None, CopyRelPath, CopyFullPath, Cut, Copy, Paste } builtin_type{BuiltinType::None};
};

struct ClipboardState {
    std::filesystem::path source_path;
    bool is_cut{false};
    bool has_content{false};
};

ClipboardState g_clipboard{};

void update_render_menu(
    const std::vector<arex::DirectoryItem>& scanned_items,
    std::vector<arex::MenuItem>& menu_items
) {
    menu_items.clear();
    menu_items.reserve(scanned_items.size() + 1);

    for (const auto& item : scanned_items) {
        menu_items.push_back(arex::MenuItem{
            .name = item.name,
            .kind = (item.type == arex::FileType::Directory) ? arex::ItemKind::Directory : arex::ItemKind::File,
            .file_size = 0
        });
    }

    menu_items.push_back(arex::MenuItem{
        .name = "[Actions...]",
        .kind = arex::ItemKind::Action,
        .file_size = 0
    });
}

void update_preview(
    const std::filesystem::path& current_dir,
    const std::vector<arex::DirectoryItem>& items,
    std::size_t selected_index,
    arex::FilePreviewer& previewer,
    std::vector<char>& preview_buffer,
    arex::RenderState& render_state,
    arex::DirectoryScanner& scanner
) {
    render_state.preview_lines.clear();
    const std::size_t max_lines = render_state.preview_height;

    if (items.empty() || selected_index >= items.size()) {
        render_state.preview_lines.emplace_back("[ Empty Directory ]");
        return;
    }

    if (selected_index == items.size()) {
        render_state.preview_lines.emplace_back("[ Folder Actions: Perform operations on current directory ]");
        return;
    }

    const auto& selected_item = items[selected_index];
    const std::filesystem::path target_path = current_dir / selected_item.name;

    if (selected_item.type == arex::FileType::Directory) {
        std::vector<arex::DirectoryItem> dir_contents;
        if (scanner.scan(target_path, dir_contents) && !dir_contents.empty()) {
            const std::size_t limit = (max_lines > 1) ? max_lines - 1 : 1;
            const std::size_t count = std::min(limit, dir_contents.size());

            for (std::size_t i = 0; i < count; ++i) {
                std::string prefix = (dir_contents[i].type == arex::FileType::Directory) ? "[DIR]  " : "       ";
                render_state.preview_lines.push_back(prefix + dir_contents[i].name);
            }

            if (dir_contents.size() > limit) {
                render_state.preview_lines.emplace_back("...");
            }
        } else {
            render_state.preview_lines.emplace_back("[ Empty Directory ]");
        }
        return;
    }

    arex::PreviewResult result;
    if (previewer.preview_file(target_path, preview_buffer, result)) {
        if (result.is_binary) {
            render_state.preview_lines.emplace_back("[ Binary File ]");
        } else if (result.is_empty) {
            render_state.preview_lines.emplace_back("[ Empty File ]");
        } else {
            render_state.preview_lines.reserve(result.lines.size());
            for (const auto line_view : result.lines) {
                render_state.preview_lines.emplace_back(line_view);
            }
        }
    } else {
        render_state.preview_lines.emplace_back("[ Error reading preview ]");
    }
}

std::vector<ActionItem> build_file_actions() {
    std::vector<ActionItem> actions = {
        {"Copy Relative Path", "", {}, true, ActionItem::BuiltinType::CopyRelPath},
        {"Copy Full Path", "", {}, true, ActionItem::BuiltinType::CopyFullPath},
        {"default (xdg-open)", "xdg-open", {"{}"}, false, ActionItem::BuiltinType::None},
        {"cat", "cat", {"{}"}, false, ActionItem::BuiltinType::None},
        {"more", "more", {"{}"}, false, ActionItem::BuiltinType::None},
        {"less", "less", {"{}"}, false, ActionItem::BuiltinType::None},
        {"cut", "", {}, true, ActionItem::BuiltinType::Cut},
        {"copy", "", {}, true, ActionItem::BuiltinType::Copy}
    };

    if (g_clipboard.has_content) {
        std::string paste_label = "paste (" + g_clipboard.source_path.filename().string() + ")";
        actions.push_back({paste_label, "", {}, true, ActionItem::BuiltinType::Paste});
    }

    return actions;
}

std::vector<ActionItem> build_directory_actions() {
    std::vector<ActionItem> actions = {
        {"cut", "", {}, true, ActionItem::BuiltinType::Cut},
        {"copy", "", {}, true, ActionItem::BuiltinType::Copy}
    };

    if (g_clipboard.has_content) {
        std::string paste_label = "paste (" + g_clipboard.source_path.filename().string() + ")";
        actions.push_back({paste_label, "", {}, true, ActionItem::BuiltinType::Paste});
    }

    actions.push_back({"ls -la", "ls", {"-la", "{}"}, false, ActionItem::BuiltinType::None});
    actions.push_back({"tree", "tree", {"{}"}, false, ActionItem::BuiltinType::None});
    actions.push_back({"tree -d", "tree", {"-d", "{}"}, false, ActionItem::BuiltinType::None});

    return actions;
}

bool copy_to_sys_clipboard(const std::string& text) {
#if defined(__APPLE__)
    std::string cmd = "echo -n \"" + text + "\" | pbcopy";
#else
    std::string cmd = "echo -n \"" + text + "\" | xclip -selection clipboard 2>/dev/null";
#endif
    const int status = ::system(cmd.c_str());
    return (status == 0);
}

void run_interactive_command(
    arex::TerminalCanvas& canvas,
    arex::KeyListener& listener,
    const ActionItem& action,
    const std::filesystem::path& target
) {
    canvas.leave_alternate_screen();
    listener.~KeyListener(); // Restore canonical termios mode

    arex::CommandConfig config{
        .name = action.label,
        .command = action.command,
        .args = action.args,
        .redirect_output_to_null = false
    };

    std::cout << "\n--- Running: " << action.command << " ---\n\n";
    const auto result = arex::ProcessRunner::execute_foreground(config, target);

    if (!result) {
        std::cerr << "\n[Command failed]\n";
    }
    std::cout << "\n[Press ENTER to return to menu]";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    new (&listener) arex::KeyListener(); // Re-enable raw termios mode
    canvas.enter_alternate_screen();
}

} // namespace

int main(int argc, char* argv[]) {
    std::filesystem::path current_dir = std::filesystem::current_path();
    if (argc > 1) {
        std::filesystem::path requested_path(argv[1]);
        std::error_code ec;
        if (std::filesystem::exists(requested_path, ec) && std::filesystem::is_directory(requested_path, ec)) {
            current_dir = std::filesystem::canonical(requested_path, ec);
        }
    }
    const std::filesystem::path initial_root = current_dir;

    arex::TerminalCanvas canvas;
    arex::KeyListener key_listener;
    arex::DirectoryScanner scanner;
    arex::FilePreviewer previewer(8, 16384);

    std::vector<arex::DirectoryItem> scanned_items;
    std::vector<char> preview_buffer;
    preview_buffer.reserve(16384);

    arex::RenderState render_state;
    render_state.preview_height = 8;

    Mode current_mode = Mode::DirectoryNavigation;
    std::vector<ActionItem> active_actions;
    std::filesystem::path action_target_path;
    std::size_t nav_selected_index = 0;
    std::size_t action_selected_index = 0;

    if (!scanner.scan(current_dir, scanned_items)) {
        std::cerr << "Failed to read launch directory: " << current_dir << "\n";
        return 1;
    }

    auto refresh_navigation_state = [&]() {
        render_state.current_path = current_dir.string();
        update_render_menu(scanned_items, render_state.menu_items);
        update_preview(current_dir, scanned_items, nav_selected_index, previewer, preview_buffer, render_state, scanner);
        if (render_state.status_message.empty()) {
            render_state.status_message = "j/k: move | Enter/l: open/actions | h/Backspace: back (Exit at launch dir) | q: quit";
        }
    };

    refresh_navigation_state();
    canvas.enter_alternate_screen();

    bool running = true;
    while (running) {
        canvas.render(render_state);

        const arex::KeyCode key = key_listener.read_key_blocking();

        if (current_mode == Mode::DirectoryNavigation) {
            const std::size_t total_menu_count = scanned_items.size() + 1;

            switch (key) {
                case arex::KeyCode::Up: {
                    if (nav_selected_index > 0) {
                        --nav_selected_index;
                        render_state.selected_index = nav_selected_index;
                        render_state.status_message.clear();
                        update_preview(current_dir, scanned_items, nav_selected_index, previewer, preview_buffer, render_state, scanner);
                    }
                    break;
                }

                case arex::KeyCode::Down: {
                    if (nav_selected_index + 1 < total_menu_count) {
                        ++nav_selected_index;
                        render_state.selected_index = nav_selected_index;
                        render_state.status_message.clear();
                        update_preview(current_dir, scanned_items, nav_selected_index, previewer, preview_buffer, render_state, scanner);
                    }
                    break;
                }

                case arex::KeyCode::Home: {
                    nav_selected_index = 0;
                    render_state.selected_index = 0;
                    render_state.status_message.clear();
                    update_preview(current_dir, scanned_items, nav_selected_index, previewer, preview_buffer, render_state, scanner);
                    break;
                }

                case arex::KeyCode::End: {
                    nav_selected_index = total_menu_count - 1;
                    render_state.selected_index = nav_selected_index;
                    render_state.status_message.clear();
                    update_preview(current_dir, scanned_items, nav_selected_index, previewer, preview_buffer, render_state, scanner);
                    break;
                }

                case arex::KeyCode::Right:
                case arex::KeyCode::Enter: {
                    if (nav_selected_index == scanned_items.size()) {
                        current_mode = Mode::DirectoryActions;
                        action_target_path = current_dir;
                        active_actions = build_directory_actions();
                        action_selected_index = 0;

                        render_state.menu_items.clear();
                        for (const auto& act : active_actions) {
                            render_state.menu_items.push_back(arex::MenuItem{
                                .name = act.label,
                                .kind = arex::ItemKind::Action,
                                .file_size = 0
                            });
                        }
                        render_state.selected_index = 0;
                        render_state.status_message = "[Folder Actions] Target: " + current_dir.filename().string() + " (Esc to cancel)";
                    } else if (nav_selected_index < scanned_items.size()) {
                        const auto& selected = scanned_items[nav_selected_index];
                        const std::filesystem::path target = current_dir / selected.name;

                        if (selected.type == arex::FileType::Directory) {
                            if (scanner.scan(target, scanned_items)) {
                                std::error_code ec;
                                current_dir = std::filesystem::canonical(target, ec);
                                nav_selected_index = 0;
                                render_state.selected_index = 0;
                                render_state.status_message.clear();
                                refresh_navigation_state();
                            }
                        } else {
                            current_mode = Mode::FileActions;
                            action_target_path = target;
                            active_actions = build_file_actions();
                            action_selected_index = 0;

                            render_state.menu_items.clear();
                            for (const auto& act : active_actions) {
                                render_state.menu_items.push_back(arex::MenuItem{
                                    .name = act.label,
                                    .kind = arex::ItemKind::Action,
                                    .file_size = 0
                                });
                            }
                            render_state.selected_index = 0;
                            render_state.status_message = "[File Actions] Target: " + selected.name + " (Esc to cancel)";
                        }
                    }
                    break;
                }

                case arex::KeyCode::Left:
                case arex::KeyCode::Backspace: {
                    if (current_dir == initial_root) {
                        running = false;
                    } else if (current_dir.has_parent_path()) {
                        const std::filesystem::path parent = current_dir.parent_path();
                        if (scanner.scan(parent, scanned_items)) {
                            current_dir = parent;
                            nav_selected_index = 0;
                            render_state.selected_index = 0;
                            render_state.status_message.clear();
                            refresh_navigation_state();
                        }
                    }
                    break;
                }

                case arex::KeyCode::Escape: {
                    running = false;
                    break;
                }

                default: break;
            }
        } 
        else {
            switch (key) {
                case arex::KeyCode::Up: {
                    if (action_selected_index > 0) {
                        --action_selected_index;
                        render_state.selected_index = action_selected_index;
                    }
                    break;
                }

                case arex::KeyCode::Down: {
                    if (action_selected_index + 1 < active_actions.size()) {
                        ++action_selected_index;
                        render_state.selected_index = action_selected_index;
                    }
                    break;
                }

                case arex::KeyCode::Enter:
                case arex::KeyCode::Right: {
                    const auto& chosen = active_actions[action_selected_index];

                    if (chosen.is_builtin) {
                        switch (chosen.builtin_type) {
                            case ActionItem::BuiltinType::CopyRelPath: {
                                std::error_code ec;
                                std::filesystem::path rel = std::filesystem::relative(action_target_path, current_dir, ec);
                                const bool ok = copy_to_sys_clipboard(rel.string());
                                render_state.status_message = ok 
                                    ? "[SUCCESS] Copied relative path: " + rel.string() 
                                    : "[FAILED] Could not copy path to clipboard";
                                break;
                            }
                            case ActionItem::BuiltinType::CopyFullPath: {
                                const bool ok = copy_to_sys_clipboard(action_target_path.string());
                                render_state.status_message = ok 
                                    ? "[SUCCESS] Copied full path: " + action_target_path.string() 
                                    : "[FAILED] Could not copy path to clipboard";
                                break;
                            }
                            case ActionItem::BuiltinType::Cut: {
                                g_clipboard = {.source_path = action_target_path, .is_cut = true, .has_content = true};
                                render_state.status_message = "[CLIPBOARD] Cut target: " + action_target_path.filename().string();
                                break;
                            }
                            case ActionItem::BuiltinType::Copy: {
                                g_clipboard = {.source_path = action_target_path, .is_cut = false, .has_content = true};
                                render_state.status_message = "[CLIPBOARD] Copied target: " + action_target_path.filename().string();
                                break;
                            }
                            case ActionItem::BuiltinType::Paste: {
                                std::error_code ec;
                                if (g_clipboard.has_content && std::filesystem::exists(g_clipboard.source_path, ec)) {
                                    std::filesystem::path dest = current_dir / g_clipboard.source_path.filename();
                                    if (g_clipboard.is_cut) {
                                        std::filesystem::rename(g_clipboard.source_path, dest, ec);
                                        if (!ec) {
                                            g_clipboard.has_content = false;
                                            render_state.status_message = "[SUCCESS] Moved " + dest.filename().string() + " here.";
                                        } else {
                                            render_state.status_message = "[ERROR] Move failed: " + ec.message();
                                        }
                                    } else {
                                        std::filesystem::copy(g_clipboard.source_path, dest, std::filesystem::copy_options::recursive, ec);
                                        if (!ec) {
                                            render_state.status_message = "[SUCCESS] Copied " + dest.filename().string() + " here.";
                                        } else {
                                            render_state.status_message = "[ERROR] Copy failed: " + ec.message();
                                        }
                                    }
                                    if (!scanner.scan(current_dir, scanned_items)) {
                                        render_state.status_message += " (Failed to rescan directory)";
                                    }
                                } else {
                                    render_state.status_message = "[ERROR] Clipboard source no longer exists.";
                                }
                                break;
                            }
                            default: break;
                        }
                    } else if (chosen.command == "xdg-open") {
                        // Background execution for default system launchers
                        arex::CommandConfig config{
                            .name = chosen.label,
                            .command = chosen.command,
                            .args = chosen.args,
                            .redirect_output_to_null = true
                        };
                        const bool launched = arex::ProcessRunner::execute_detached(config, action_target_path);
                        render_state.status_message = launched 
                            ? "[LAUNCHED] Opened: " + action_target_path.filename().string() 
                            : "[ERROR] Failed to open file.";
                    } else {
                        // Foreground interactive handover for core commands cat, less, tree, ls, etc.
                        // note - all the commands are presumed to be installed on every distro
                        // TODO: add handling of windows where there might be no linux aliases
                        run_interactive_command(canvas, key_listener, chosen, action_target_path);
                        render_state.status_message = "[FINISHED] Exited action: " + chosen.label;
                    }

                    current_mode = Mode::DirectoryNavigation;
                    render_state.selected_index = nav_selected_index;
                    refresh_navigation_state();
                    break;
                }

                case arex::KeyCode::Escape:
                case arex::KeyCode::Left:
                case arex::KeyCode::Backspace: {
                    current_mode = Mode::DirectoryNavigation;
                    render_state.selected_index = nav_selected_index;
                    render_state.status_message.clear();
                    refresh_navigation_state();
                    break;
                }

                default: break;
            }
        }
    }

    canvas.leave_alternate_screen();
    return 0;
}