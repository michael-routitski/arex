#ifndef AREX_ACTIONS_PASTE_HPP
#define AREX_ACTIONS_PASTE_HPP

#include "action.hpp"
#include "protected_paths.hpp"

#include <system_error>

namespace arex {

inline bool paste_action(
    ActionContext& context,
    const ActionItem&
) {
    std::error_code ec;
    if (!context.clipboard.has_content) {
        context.render_state.status_message =
            "[ERROR] Clipboard is empty.";
        return false;
    }
    if (is_protected_top_level_directory(context.clipboard.source_path)) {
        context.clipboard.has_content = false;
        context.render_state.status_message =
            "[ERROR] Cannot paste a protected top-level directory.";
        return false;
    }
    if (is_protected_top_level_directory(context.current_dir)) {
        context.render_state.status_message =
            "[ERROR] Cannot paste into a protected top-level directory.";
        return false;
    }
    if (!std::filesystem::exists(context.clipboard.source_path, ec)) {
        context.render_state.status_message =
            "[ERROR] Clipboard source no longer exists.";
        return false;
    }

    const std::filesystem::path dest =
        context.current_dir / context.clipboard.source_path.filename();

    if (context.clipboard.is_cut) {
        std::filesystem::rename(
            context.clipboard.source_path,
            dest,
            ec
        );
        if (!ec) {
            context.clipboard.has_content = false;
            context.render_state.status_message =
                "[SUCCESS] Moved " + dest.filename().string() + " here.";
        } else {
            ec.clear();
            std::filesystem::copy(
                context.clipboard.source_path,
                dest,
                std::filesystem::copy_options::recursive |
                    std::filesystem::copy_options::copy_symlinks,
                ec
            );
            if (!ec) {
                std::filesystem::remove_all(
                    context.clipboard.source_path,
                    ec
                );
                if (!ec) {
                    context.clipboard.has_content = false;
                    context.render_state.status_message =
                        "[SUCCESS] Moved " +
                        dest.filename().string() + " here.";
                } else {
                    context.render_state.status_message =
                        "[ERROR] Move cleanup failed: " + ec.message();
                    return false;
                }
            } else {
                context.render_state.status_message =
                    "[ERROR] Move failed: " + ec.message();
                return false;
            }
        }
    } else {
        std::filesystem::copy(
            context.clipboard.source_path,
            dest,
            std::filesystem::copy_options::recursive |
                std::filesystem::copy_options::copy_symlinks,
            ec
        );
        if (!ec) {
            context.render_state.status_message =
                "[SUCCESS] Copied " +
                dest.filename().string() + " here.";
        } else {
            context.render_state.status_message =
                "[ERROR] Copy failed: " + ec.message();
            return false;
        }
    }

    if (!context.scanner.scan(context.current_dir, context.scanned_items)) {
        context.render_state.status_message +=
            " (Failed to rescan directory)";
    }
    return true;
}

} // namespace arex

#endif // AREX_ACTIONS_PASTE_HPP
