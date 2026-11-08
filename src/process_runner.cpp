#include <arex/process_runner.hpp>

#include <cstdlib>
#include <vector>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

namespace arex {

bool ProcessRunner::execute_detached(
    const CommandConfig& config,
    const std::filesystem::path& target_path
) {
    const std::string path_str = target_path.string();

#if defined(_WIN32)
    std::string cmdline = config.command;
    bool replaced = false;

    for (const auto& arg : config.args) {
        cmdline += " ";
        if (arg == "{}") {
            cmdline += "\"" + path_str + "\"";
            replaced = true;
        } else {
            cmdline += arg;
        }
    }

    if (!replaced) {
        cmdline += " \"" + path_str + "\"";
    }

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    const BOOL success = CreateProcessA(
        NULL,
        cmdline.data(),
        NULL,
        NULL,
        FALSE,
        DETACHED_PROCESS | CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
#else
    std::vector<std::string> formatted_args;
    formatted_args.reserve(config.args.size() + 2);
    formatted_args.push_back(config.command);

    bool replaced = false;
    for (const auto& arg : config.args) {
        if (arg == "{}") {
            formatted_args.push_back(path_str);
            replaced = true;
        } else {
            formatted_args.push_back(arg);
        }
    }

    if (!replaced) {
        formatted_args.push_back(path_str);
    }

    std::vector<char*> argv_ptrs;
    argv_ptrs.reserve(formatted_args.size() + 1);
    for (auto& arg : formatted_args) {
        argv_ptrs.push_back(arg.data());
    }
    argv_ptrs.push_back(nullptr);

    const pid_t pid = ::fork();

    if (pid < 0) {
        return false;
    }

    if (pid == 0) {
        const pid_t grand_child_pid = ::fork();

        if (grand_child_pid > 0) {
            ::_exit(0);
        }

        if (grand_child_pid == 0) {
            if (config.redirect_output_to_null) {
                const int dev_null = ::open("/dev/null", O_RDWR);
                if (dev_null != -1) {
                    ::dup2(dev_null, STDIN_FILENO);
                    ::dup2(dev_null, STDOUT_FILENO);
                    ::dup2(dev_null, STDERR_FILENO);
                    ::close(dev_null);
                }
            }

            ::execvp(argv_ptrs[0], argv_ptrs.data());
            ::_exit(127);
        }

        ::_exit(1);
    }

    ::waitpid(pid, nullptr, 0);
    return true;
#endif
}

bool ProcessRunner::execute_foreground(
    const CommandConfig& config,
    const std::filesystem::path& target_path
) {
    const std::string path_str = target_path.string();

#if defined(_WIN32)
    std::string cmdline = config.command;
    bool replaced = false;
    for (const auto& arg : config.args) {
        cmdline += " ";
        if (arg == "{}") {
            cmdline += "\"" + path_str + "\"";
            replaced = true;
        } else {
            cmdline += arg;
        }
    }
    if (!replaced) {
        cmdline += " \"" + path_str + "\"";
    }

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    const BOOL success = CreateProcessA(NULL, cmdline.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    if (success) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
#else
    std::vector<std::string> formatted_args;
    formatted_args.reserve(config.args.size() + 2);
    formatted_args.push_back(config.command);

    bool replaced = false;
    for (const auto& arg : config.args) {
        if (arg == "{}") {
            formatted_args.push_back(path_str);
            replaced = true;
        } else {
            formatted_args.push_back(arg);
        }
    }

    if (!replaced) {
        formatted_args.push_back(path_str);
    }

    std::vector<char*> argv_ptrs;
    argv_ptrs.reserve(formatted_args.size() + 1);
    for (auto& arg : formatted_args) {
        argv_ptrs.push_back(arg.data());
    }
    argv_ptrs.push_back(nullptr);

    const pid_t pid = ::fork();
    if (pid < 0) {
        return false;
    }

    if (pid == 0) {
        // Child inherits stdout/stdin/stderr directly from parent terminal
        ::execvp(argv_ptrs[0], argv_ptrs.data());
        ::_exit(127);
    }

    int status = 0;
    ::waitpid(pid, &status, 0);
    return WIFEXITED(status) && (WEXITSTATUS(status) == 0);
#endif
}

} // namespace arex