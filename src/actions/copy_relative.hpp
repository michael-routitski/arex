#ifndef AREX_ACTIONS_COPY_RELATIVE_HPP
#define AREX_ACTIONS_COPY_RELATIVE_HPP

#include "action.hpp"
#include "clipboard.hpp"

namespace arex {

inline bool copy_relative_action(
    ActionContext& context,
    const ActionItem&
) {
    std::error_code ec;
    const std::filesystem::path base =
        context.target == context.current_dir
            ? context.current_dir.parent_path()
            : context.current_dir;
    std::filesystem::path relative =
        std::filesystem::relative(context.target, base, ec);
    if (ec) {
        relative = context.target.filename();
    }
    const bool success = copy_to_system_clipboard(relative.string());
    context.render_state.status_message = success
        ? "[SUCCESS] Copied relative path: " + relative.string()
        : "[FAILED] Could not copy path to clipboard";
    return success;
}

} // namespace arex

#endif // AREX_ACTIONS_COPY_RELATIVE_HPP
