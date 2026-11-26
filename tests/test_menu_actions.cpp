#include <arex/menu_actions.hpp>

#include <cassert>

int main() {
    arex::TerminalCanvas canvas;
    arex::kernels::KeyboardKernel keyboard_kernel;
    arex::DirectoryScanner scanner;
    arex::MenuNavigator navigator;
    arex::MenuActions actions(
        canvas,
        keyboard_kernel,
        scanner,
        navigator.current_dir(),
        navigator.scanned_items(),
        navigator.selected_index(),
        navigator.render_state()
    );

    const std::filesystem::path target =
        std::filesystem::current_path() / "test-file.txt";
    actions.open_file(target);
    assert(navigator.render_state().menu_items.size() >= 8);
    return 0;
}
