#include <arex/menu_app.hpp>

#include <arex/kernels/terminal_kernel.hpp>

namespace arex {

MenuApp::MenuApp()
    : _actions(
          _navigator.canvas(),
          _navigator.keyboard_kernel(),
          _navigator.scanner(),
          _navigator.current_dir(),
          _navigator.scanned_items(),
          _navigator.selected_index(),
          _navigator.render_state()
      ) {}

int MenuApp::run(int argc, char** argv) {
    if (!_navigator.initialize(argc, argv)) {
        kernels::TerminalKernel terminal_kernel;
        const std::string failure_message =
            "Failed to read launch directory: " +
            _navigator.current_dir().string();
        kernels::TerminalLine error_line{
            .contents = failure_message.data(),
            .contents_size = failure_message.size(),
            .prefix = kernels::terminal_constants::ERROR_LABEL.data(),
            .prefix_size = kernels::terminal_constants::ERROR_LABEL.size(),
            .suffix = nullptr,
            .suffix_size = 0,
            .type = kernels::TerminalLineType::Error,
            .brightness = kernels::TerminalBrightness::Default,
            .selected = false
        };
        terminal_kernel.write_line(error_line);
        return 1;
    }

    _navigator.canvas().enter_alternate_screen();

    while (_navigator.running()) {
        _navigator.render();

        const kernels::KeyCode key =
            _navigator.keyboard_kernel().read_key_blocking();
        const MenuEvent event =
            _mode == MenuMode::Navigation
                ? _navigator.handle_key(key)
                : _actions.handle_key(key);

        if (event.type == MenuEventType::OpenFileActions) {
            _actions.open_file(event.target);
            _mode = MenuMode::FileActions;
        } else if (event.type == MenuEventType::OpenDirectoryActions) {
            _actions.open_directory(event.target);
            _mode = MenuMode::DirectoryActions;
        } else if (event.type == MenuEventType::CloseActions) {
            _navigator.refresh();
            _mode = MenuMode::Navigation;
        } else if (event.type == MenuEventType::Exit) {
            _navigator.leave_alternate_screen();
            break;
        }
    }

    _navigator.leave_alternate_screen();
    return 0;
}

} // namespace arex
