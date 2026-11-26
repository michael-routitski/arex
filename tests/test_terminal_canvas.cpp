#include <arex/terminal_canvas.hpp>

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
    std::cout << "Terminal canvas output capture is not implemented on Windows\n";
    return 0;
#else
    char capture_path[] = "/tmp/arex-terminal-canvas-test-XXXXXX";
    const int capture_fd = ::mkstemp(capture_path);
    if (capture_fd < 0) {
        std::cerr << "Failed to create terminal canvas capture\n";
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

    arex::TerminalCanvas canvas;
    arex::RenderState state;
    state.preview_overflow = true;
    for (std::size_t i = 0; i < 10; ++i) {
        state.preview_lines.push_back("preview-" + std::to_string(i));
    }
    state.menu_items.reserve(30);
    state.menu_items.push_back(
        {.name = ".gitignore_global", .kind = arex::ItemKind::File}
    );
    state.menu_items.push_back(
        {.name = "sync-excludes.sh", .kind = arex::ItemKind::File}
    );
    state.menu_items.push_back(
        {.name = "logs", .kind = arex::ItemKind::Directory}
    );
    for (std::size_t i = 0; i < 26; ++i) {
        state.menu_items.push_back(
            {.name = "item-" + std::to_string(i), .kind = arex::ItemKind::File}
        );
    }
    state.menu_items.push_back(
        {
            .name = "Dir Actions ...",
            .kind = arex::ItemKind::Action,
            .file_size = 0,
            .action_label = arex::kernels::terminal_constants::ROOT_ACTION_LABEL
        }
    );
    state.selected_index = 11;
    canvas.render(state);

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

    const bool viewport_format_valid =
        frame.find(".gitignore_global") == std::string::npos &&
        frame.find("[FILE] item-8") != std::string::npos &&
        frame.find("[FILE] item-12") == std::string::npos &&
        frame.find("[FILE]  ") == std::string::npos;
    const bool preview_height_valid =
        frame.find("preview-0") != std::string::npos &&
        frame.find("preview-5") != std::string::npos &&
        frame.find("preview-6") == std::string::npos &&
        frame.find("│ \x1b[2m\x1b[38;2;160;160;160m") != std::string::npos &&
        frame.find("│ \x1b[2m\x1b[38;2;96;96;96m") != std::string::npos;
    const bool selection_format_valid =
        frame.find("\x1b[1m") != std::string::npos &&
        frame.find("\x1b[7m") != std::string::npos &&
        frame.find("\x1b[38;2;0;72;36m") != std::string::npos &&
        frame.find("\x1b[38;2;0;128;64m") != std::string::npos;

    if (!viewport_format_valid || !preview_height_valid || !selection_format_valid) {
        std::cerr << "Terminal menu item labels do not match the expected format\n";
        return 1;
    }

    return 0;
#endif
}
