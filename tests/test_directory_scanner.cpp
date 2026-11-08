#include <arex/directory_scanner.hpp>
#include <cassert>
#include <iostream>

int main() {
    arex::DirectoryScanner scanner;
    std::vector<arex::DirectoryItem> items;

    const bool success = scanner.scan(".", items);
    assert(success && "Directory scan failed");
    assert(!items.empty() && "Directory listing should not be empty");

    std::cout << "[Scanner Test] Discovered " << items.size() << " entries:\n";
    for (const auto& item : items) {
        std::cout << "  " << (item.type == arex::FileType::Directory ? "[DIR]  " : "[FILE] ")
                  << item.name << "\n";
    }

    std::cout << "[Test Passed] test_directory_scanner\n";
    return 0;
}