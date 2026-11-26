#ifndef AREX_ACTIONS_TREE_DIRECTORY_HPP
#define AREX_ACTIONS_TREE_DIRECTORY_HPP

#include "action.hpp"
#include "command.hpp"

namespace arex {

inline ActionItem make_tree_directory_action() {
    return {
        "tree -d",
        "tree",
        {"-d", "{}"},
        ActionExecution::Foreground,
        execute_foreground_command
    };
}

} // namespace arex

#endif // AREX_ACTIONS_TREE_DIRECTORY_HPP
