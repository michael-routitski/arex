#ifndef AREX_ACTIONS_CAT_HPP
#define AREX_ACTIONS_CAT_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_cat_action() {
    return {
        "cat",
        "cat",
        {"{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_CAT_HPP
