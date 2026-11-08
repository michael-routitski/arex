#ifndef AREX_PROCESS_RUNNER_HPP
#define AREX_PROCESS_RUNNER_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace arex {

struct CommandConfig {
    std::string name;
    std::string command;
    std::vector<std::string> args;
    bool redirect_output_to_null{true};
};

class ProcessRunner {
public:
    ProcessRunner() = default;
    ~ProcessRunner() = default;

    ProcessRunner(const ProcessRunner&) = delete;
    ProcessRunner& operator=(const ProcessRunner&) = delete;

    ProcessRunner(ProcessRunner&&) noexcept = default;
    ProcessRunner& operator=(ProcessRunner&&) noexcept = default;

    /// Spawns a background command asynchronously without blocking the TUI event loop.
    [[nodiscard]] static bool execute_detached(
        const CommandConfig& config,
        const std::filesystem::path& target_path
    );

    /// Spawns an interactive command in the foreground, inheriting stdio,
    /// and blocks until the child process completes.
    [[nodiscard]] static bool execute_foreground(
        const CommandConfig& config,
        const std::filesystem::path& target_path
    );
};

} // namespace arex

#endif // AREX_PROCESS_RUNNER_HPP