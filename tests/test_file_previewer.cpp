#include <arex/file_previewer.hpp>
#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    const std::filesystem::path test_file = "test_preview.txt";
    {
        std::ofstream out(test_file);
        for (int i = 1; i <= 30; ++i) {
            out << "Line " << i << ": High-performance C++ preview\n";
        }
    }

    arex::FilePreviewer previewer(5, 1024);
    std::vector<char> buffer;
    arex::PreviewResult result;

    const bool success = previewer.preview_file(test_file, buffer, result);
    std::filesystem::remove(test_file);

    assert(success && "File preview failed");
    assert(!result.is_binary && "File should not be detected as binary");
    assert(result.lines.size() == 5 && "Should preview exactly 5 lines");

    std::cout << "[Previewer Test] Previewed " << result.lines.size() << " lines:\n";
    for (const auto line : result.lines) {
        std::cout << "  " << line << "\n";
    }

    std::cout << "[Test Passed] test_file_previewer\n";
    return 0;
}