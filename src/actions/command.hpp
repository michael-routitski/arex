#ifndef AREX_ACTIONS_COMMAND_HPP
#define AREX_ACTIONS_COMMAND_HPP

#include "action.hpp"

#include <arex/kernels/process_kernel.hpp>
#include <arex/kernels/terminal_kernel.hpp>

#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace arex {

inline bool execute_detached_command(
    ActionContext& context,
    const ActionItem& action
) {
    kernels::CommandConfig config{
        .name = action.label,
        .command = action.command,
        .args = action.args,
        .redirect_output_to_null = true
    };
    const bool launched = kernels::ProcessKernel::execute_detached(
        config,
        context.target
    );
    context.render_state.status_message = launched
        ? "[LAUNCHED] Opened: " + context.target.filename().string()
        : "[ERROR] Failed to open file.";
    return launched;
}

inline bool execute_foreground_command(
    ActionContext& context,
    const ActionItem& action
) {
    kernels::TerminalKernel terminal_kernel;
    context.canvas.leave_alternate_screen();
    context.keyboard_kernel.~KeyboardKernel();

    kernels::CommandConfig config{
        .name = action.label,
        .command = action.command,
        .args = action.args,
        .redirect_output_to_null = false
    };

    const std::string running_message = "Running: " + action.command;
    kernels::TerminalLine running_line{
        .contents = running_message.data(),
        .contents_size = running_message.size(),
        .prefix = kernels::terminal_constants::INFO_PREFIX.data(),
        .prefix_size = kernels::terminal_constants::INFO_PREFIX.size(),
        .suffix = nullptr,
        .suffix_size = 0,
        .type = kernels::TerminalLineType::Info,
        .brightness = kernels::TerminalBrightness::Default,
        .selected = false
    };
    terminal_kernel.write_line(running_line);

    const bool success =
        kernels::ProcessKernel::execute_foreground(config, context.target);
    if (!success) {
        static constexpr std::string_view error_message = "Command failed";
        kernels::TerminalLine error_line{
            .contents = error_message.data(),
            .contents_size = error_message.size(),
            .prefix = kernels::terminal_constants::ERROR_LABEL.data(),
            .prefix_size = kernels::terminal_constants::ERROR_LABEL.size(),
            .suffix = nullptr,
            .suffix_size = 0,
            .type = kernels::TerminalLineType::Error,
            .brightness = kernels::TerminalBrightness::Default,
            .selected = false
        };
        terminal_kernel.write_line(error_line);
    }

    static constexpr std::string_view prompt_message =
        "Press ENTER to return to menu";
    kernels::TerminalLine prompt_line{
        .contents = prompt_message.data(),
        .contents_size = prompt_message.size(),
        .prefix = kernels::terminal_constants::INFO_PREFIX.data(),
        .prefix_size = kernels::terminal_constants::INFO_PREFIX.size(),
        .suffix = nullptr,
        .suffix_size = 0,
        .type = kernels::TerminalLineType::Info,
        .brightness = kernels::TerminalBrightness::Default,
        .selected = false
    };
    terminal_kernel.write_line(prompt_line);
    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n'
    );

    new (&context.keyboard_kernel) kernels::KeyboardKernel();
    context.canvas.enter_alternate_screen();
    context.render_state.status_message =
        "[FINISHED] Exited action: " + action.label;
    return success;
}

} // namespace arex

#endif // AREX_ACTIONS_COMMAND_HPP
