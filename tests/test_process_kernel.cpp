#include <arex/kernels/process_kernel.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

int main() {
    const std::filesystem::path dummy_file = "test_run.txt";
    {
        std::ofstream out(dummy_file);
        out << "hello";
    }

    arex::kernels::CommandConfig config{
        .name = "Touch File",
#if defined(_WIN32)
        .command = "cmd.exe",
        .args = {"/c", "type", "{}"}
#else
        .command = "cat",
        .args = {"{}"}
#endif
    };

    const bool success =
        arex::kernels::ProcessKernel::execute_detached(config, dummy_file);
    std::filesystem::remove(dummy_file);

    assert(success && "ProcessKernel execution failed");
    std::cout << "[Test Passed] test_process_kernel\n";
    return 0;
}
