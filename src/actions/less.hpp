#ifndef AREX_ACTIONS_LESS_HPP
#define AREX_ACTIONS_LESS_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_less_action() {
    return {
        "less",
        "less",
        {"{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_LESS_HPP
