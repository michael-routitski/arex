#ifndef AREX_ACTIONS_TREE_HPP
#define AREX_ACTIONS_TREE_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_tree_action() {
    return {
        "tree",
        "tree",
        {"{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_TREE_HPP
