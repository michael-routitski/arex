#ifndef AREX_ACTIONS_DEFAULT_HPP
#define AREX_ACTIONS_DEFAULT_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_default_action() {
    return {
        "default",
        "xdg-open",
        {"{}"},
        ActionExecution::Detached,
        execute_detached_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_DEFAULT_HPP
