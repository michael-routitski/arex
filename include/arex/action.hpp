#ifndef AREX_ACTION_HPP
#define AREX_ACTION_HPP

#include <arex/directory_scanner.hpp>
#include <arex/kernels/keyboard_kernel.hpp>
#include <arex/terminal_canvas.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace arex {

struct ActionClipboard {
    std::filesystem::path source_path;
    bool is_cut{false};
    bool has_content{false};
};

struct ActionContext {
    std::filesystem::path& target;
    std::filesystem::path& current_dir;
    ActionClipboard& clipboard;
    TerminalCanvas& canvas;
    kernels::KeyboardKernel& keyboard_kernel;
    DirectoryScanner& scanner;
    std::vector<DirectoryItem>& scanned_items;
    RenderState& render_state;
};

enum class ActionExecution : std::uint8_t {
    Builtin = 0,
    Foreground,
    Detached
};

struct ActionItem;

using ActionExecutor = bool (*)(ActionContext&, const ActionItem&);

struct ActionItem {
    std::string label;
    std::string command;
    std::vector<std::string> args;
    ActionExecution execution{ActionExecution::Builtin};
    ActionExecutor execute{nullptr};
};

} // namespace arex

#endif // AREX_ACTION_HPP
