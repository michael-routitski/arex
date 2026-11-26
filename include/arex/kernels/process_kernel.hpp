#ifndef AREX_KERNELS_PROCESS_KERNEL_HPP
#define AREX_KERNELS_PROCESS_KERNEL_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace arex::kernels {

struct CommandConfig {
    std::string name;
    std::string command;
    std::vector<std::string> args;
    bool redirect_output_to_null{true};
};

class ProcessKernel {
public:
    ProcessKernel() = default;
    ~ProcessKernel() = default;

    ProcessKernel(const ProcessKernel&) = delete;
    ProcessKernel& operator=(const ProcessKernel&) = delete;

    ProcessKernel(ProcessKernel&&) noexcept = default;
    ProcessKernel& operator=(ProcessKernel&&) noexcept = default;

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

} // namespace arex::kernels

#endif // AREX_KERNELS_PROCESS_KERNEL_HPP
