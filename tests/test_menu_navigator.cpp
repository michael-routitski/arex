#include <arex/menu_navigator.hpp>

#include <cassert>

int main() {
    arex::MenuNavigator navigator;
    const bool initialized = navigator.initialize(1, nullptr);
    assert(initialized && "MenuNavigator should initialize in the workspace");
    assert(navigator.running() && "MenuNavigator should start running");
    assert(navigator.render_state().menu_items.size() >= 1);
    return 0;
}
