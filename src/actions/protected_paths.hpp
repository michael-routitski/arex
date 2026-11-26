#ifndef AREX_ACTIONS_PROTECTED_PATHS_HPP
#define AREX_ACTIONS_PROTECTED_PATHS_HPP

#include <filesystem>
#include <system_error>

namespace arex {

inline bool is_protected_top_level_directory(
    const std::filesystem::path& target
) {
    std::error_code ec;
    std::filesystem::path absolute =
        std::filesystem::weakly_canonical(target, ec);
    if (ec) {
        absolute = target.lexically_normal();
    }
    if (!absolute.is_absolute()) {
        absolute = std::filesystem::absolute(absolute, ec).lexically_normal();
        if (ec) {
            return false;
        }
    }

    absolute = absolute.lexically_normal();
    const std::filesystem::path root = absolute.root_path();
    if (absolute == root) {
        return true;
    }
    if (absolute.parent_path() == root) {
        std::error_code directory_ec;
        return std::filesystem::is_directory(absolute, directory_ec);
    }
    return false;
}

} // namespace arex

#endif // AREX_ACTIONS_PROTECTED_PATHS_HPP
