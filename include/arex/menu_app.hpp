#ifndef AREX_MENU_APP_HPP
#define AREX_MENU_APP_HPP

#include <arex/menu_actions.hpp>
#include <arex/menu_navigator.hpp>

namespace arex {

enum class MenuMode : std::uint8_t {
    Navigation = 0,
    FileActions,
    DirectoryActions
};

class MenuApp {
public:
    MenuApp();
    ~MenuApp() = default;

    int run(int argc, char** argv);

private:
    MenuNavigator _navigator;
    MenuActions _actions;
    MenuMode _mode{MenuMode::Navigation};
};

} // namespace arex

#endif // AREX_MENU_APP_HPP
