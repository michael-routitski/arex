#ifndef AREX_ACTIONS_LS_HPP
#define AREX_ACTIONS_LS_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_ls_action() {
    return {
        "ls -la",
        "ls",
        {"-la", "{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_LS_HPP
