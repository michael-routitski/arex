#include <arex/process_runner.hpp>
#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    const std::filesystem::path dummy_file = "test_run.txt";
    {
        std::ofstream out(dummy_file);
        out << "hello";
    }

    arex::CommandConfig config{
        .name = "Touch File",
#if defined(_WIN32)
        .command = "cmd.exe",
        .args = {"/c", "type", "{}"}
#else
        .command = "cat",
        .args = {"{}"}
#endif
    };

    const bool success = arex::ProcessRunner::execute_detached(config, dummy_file);
    std::filesystem::remove(dummy_file);

    assert(success && "ProcessRunner execution failed");
    std::cout << "[Test Passed] test_process_runner\n";
    return 0;
}