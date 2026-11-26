#include <arex/kernels/terminal_kernel.hpp>

#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#if !defined(_WIN32)
#include <fcntl.h>
#include <unistd.h>
#endif

int main() {
#if defined(_WIN32)
    std::cout << "Terminal kernel output capture is not implemented on Windows\n";
    return 0;
#else
    static constexpr std::string_view filename = ".gitignore_global";
    static constexpr std::string_view dirname = "logs";
    static constexpr std::string_view action = "Dir Actions ...";
    static constexpr std::string_view dimmer_action = "More Actions ...";
    static constexpr std::string_view copy_path =
        "Copy /very/long/project/path/with/a/really/long/directory/structure/filename.txt";

    const arex::kernels::TerminalLine lines_data[] = {
        {
            .contents = filename.data(),
            .contents_size = filename.size(),
            .prefix = nullptr,
            .prefix_size = 0,
            .suffix = nullptr,
            .suffix_size = 0,
            .type = arex::kernels::TerminalLineType::Filename,
            .brightness = arex::kernels::TerminalBrightness::Default,
            .selected = false
        },
        {
            .contents = dirname.data(),
            .contents_size = dirname.size(),
            .prefix = nullptr,
            .prefix_size = 0,
            .suffix = nullptr,
            .suffix_size = 0,
            .type = arex::kernels::TerminalLineType::Dirname,
            .brightness = arex::kernels::TerminalBrightness::Dim,
            .selected = false
        },
        {
            .contents = action.data(),
            .contents_size = action.size(),
            .prefix = arex::kernels::terminal_constants::ACTION_LABEL.data(),
            .prefix_size = arex::kernels::terminal_constants::ACTION_LABEL.size(),
            .suffix = nullptr,
            .suffix_size = 0,
            .type = arex::kernels::TerminalLineType::Action,
            .brightness = arex::kernels::TerminalBrightness::Highlight,
            .selected = false
        },
        {
            .contents = dimmer_action.data(),
            .contents_size = dimmer_action.size(),
            .prefix = arex::kernels::terminal_constants::ACTION_LABEL.data(),
            .prefix_size = arex::kernels::terminal_constants::ACTION_LABEL.size(),
            .suffix = nullptr,
            .suffix_size = 0,
            .type = arex::kernels::TerminalLineType::Action,
            .brightness = arex::kernels::TerminalBrightness::Dimmer,
            .selected = false
        },
        {
            .contents = copy_path.data(),
            .contents_size = copy_path.size(),
            .prefix = arex::kernels::terminal_constants::ACTION_LABEL.data(),
            .prefix_size = arex::kernels::terminal_constants::ACTION_LABEL.size(),
            .suffix = nullptr,
            .suffix_size = 0,
            .type = arex::kernels::TerminalLineType::Action,
            .brightness = arex::kernels::TerminalBrightness::Default,
            .selected = false
        }
    };

    char capture_path[] = "/tmp/arex-terminal-kernel-test-XXXXXX";
    const int capture_fd = ::mkstemp(capture_path);
    if (capture_fd < 0) {
        std::cerr << "Failed to create terminal kernel capture\n";
        return 1;
    }

    const int saved_stdout = ::dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
        ::close(capture_fd);
        ::unlink(capture_path);
        std::cerr << "Failed to duplicate stdout\n";
        return 1;
    }

    if (::dup2(capture_fd, STDOUT_FILENO) < 0) {
        ::close(saved_stdout);
        ::close(capture_fd);
        ::unlink(capture_path);
        std::cerr << "Failed to redirect stdout\n";
        return 1;
    }

    arex::kernels::TerminalKernel kernel;
    kernel.write_frame(lines_data, 80);

    if (::dup2(saved_stdout, STDOUT_FILENO) < 0) {
        ::close(saved_stdout);
        ::close(capture_fd);
        ::unlink(capture_path);
        std::cerr << "Failed to restore stdout\n";
        return 1;
    }
    ::close(saved_stdout);
    ::close(capture_fd);

    std::ifstream capture(capture_path);
    const std::string frame(
        (std::istreambuf_iterator<char>(capture)),
        std::istreambuf_iterator<char>()
    );
    ::unlink(capture_path);

    if (frame.find("[FILE] .gitignore_global") == std::string::npos ||
        frame.find("[DIR] logs/") == std::string::npos ||
        frame.find("[ACTION] Dir Actions ...") == std::string::npos ||
        frame.find("[ACTION] More Actions ...") == std::string::npos ||
        frame.find("[ACTION] Copy /very/") == std::string::npos ||
        frame.find("filename.txt") == std::string::npos ||
        frame.find("...") == std::string::npos ||
        frame.find("\x1b[1m") == std::string::npos ||
        frame.find("\x1b[38;2;0;128;128m") == std::string::npos ||
        frame.find("\x1b[38;2;96;96;0m") == std::string::npos) {
        std::cerr << "Terminal kernel output does not match expected formatting\n";
        return 1;
    }

    return 0;
#endif
}
