#ifndef AREX_ACTIONS_MORE_HPP
#define AREX_ACTIONS_MORE_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_more_action() {
    return {
        "more",
        "more",
        {"{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_MORE_HPP
