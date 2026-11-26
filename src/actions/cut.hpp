#ifndef AREX_ACTIONS_CUT_HPP
#define AREX_ACTIONS_CUT_HPP

#include "action.hpp"
#include "protected_paths.hpp"

namespace arex {

inline bool cut_action(
    ActionContext& context,
    const ActionItem&
) {
    if (is_protected_top_level_directory(context.target)) {
        context.render_state.status_message =
            "[ERROR] Cannot cut a protected top-level directory.";
        return false;
    }
    context.clipboard = {
        .source_path = context.target,
        .is_cut = true,
        .has_content = true
    };
    context.render_state.status_message =
        "[CLIPBOARD] Cut target: " +
        context.target.filename().string();
    return true;
}

} // namespace arex

#endif // AREX_ACTIONS_CUT_HPP
