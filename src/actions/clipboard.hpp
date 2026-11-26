#ifndef AREX_ACTIONS_CLIPBOARD_HPP
#define AREX_ACTIONS_CLIPBOARD_HPP

#include <cstdlib>
#include <string>

namespace arex {

inline bool copy_to_system_clipboard(const std::string& text) {
#if defined(__APPLE__)
    std::string command = "echo -n \"" + text + "\" | pbcopy";
#else
    std::string command =
        "echo -n \"" + text + "\" | xclip -selection clipboard 2>/dev/null";
#endif
    return ::system(command.c_str()) == 0;
}

} // namespace arex

#endif // AREX_ACTIONS_CLIPBOARD_HPP
