#ifndef AREX_ACTIONS_COPY_FULL_HPP
#define AREX_ACTIONS_COPY_FULL_HPP

#include "action.hpp"
#include "clipboard.hpp"

namespace arex {

inline bool copy_full_action(
    ActionContext& context,
    const ActionItem&
) {
    const std::string full_path =
        context.target.lexically_normal().string();
    const bool success = copy_to_system_clipboard(full_path);
    context.render_state.status_message = success
        ? "[SUCCESS] Copied full path: " + full_path
        : "[FAILED] Could not copy path to clipboard";
    return success;
}

} // namespace arex

#endif // AREX_ACTIONS_COPY_FULL_HPP
