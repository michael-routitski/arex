#include <arex/file_previewer.hpp>

#include <algorithm>
#include <cstring>
#include <fcntl.h>

#if defined(_WIN32)
    #include <io.h>
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace arex {

namespace {

// for fast checking just inspect the first 512 bytes for null bytes
inline bool detect_binary(const char* data, std::size_t len) {
    const std::size_t sample_size = std::min<std::size_t>(len, 512);
    return std::memchr(data, 0, sample_size) != nullptr;
}

} // namespace

FilePreviewer::FilePreviewer(std::size_t max_lines, std::size_t max_bytes)
    : m_max_lines(max_lines), m_max_bytes(max_bytes) {}

bool FilePreviewer::preview_file(
    const std::filesystem::path& path,
    std::vector<char>& out_buffer,
    PreviewResult& out_result
) {
    out_result.lines.clear();
    out_result.is_binary = false;
    out_result.is_empty = false;
    out_result.bytes_read = 0;

#if defined(_WIN32)
    // Windows implementation using ReadFile for small fixed buffers
    // TODO: need to test this properly
    const HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    out_buffer.resize(m_max_bytes);
    DWORD bytesRead = 0;
    const BOOL success = ReadFile(hFile, out_buffer.data(), static_cast<DWORD>(m_max_bytes), &bytesRead, NULL);
    CloseHandle(hFile);

    if (!success || bytesRead == 0) {
        out_result.is_empty = true;
        return true;
    }

    const std::size_t file_size = static_cast<std::size_t>(bytesRead);
    const char* mapped_data = out_buffer.data();
#else
    // POSIX mmap implementation for zero-copy kernel page mapping
    const int fd = ::open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        return false;
    }

    struct stat st{};
    if (::fstat(fd, &st) == -1 || st.st_size == 0) {
        ::close(fd);
        out_result.is_empty = true;
        return true;
    }

    const std::size_t file_size = static_cast<std::size_t>(st.st_size);
    const std::size_t map_size = std::min(file_size, m_max_bytes);

    void* map_ptr = ::mmap(nullptr, map_size, PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd); // close asap

    if (map_ptr == MAP_FAILED) {
        return false;
    }

    const char* mapped_data = static_cast<const char*>(map_ptr);
#endif

    out_result.bytes_read = std::min(file_size, m_max_bytes);

    // 1. Instant Binary Detection
    if (detect_binary(mapped_data, out_result.bytes_read)) {
        out_result.is_binary = true;
#if !defined(_WIN32)
        ::munmap(map_ptr, map_size);
#endif
        return true;
    }

#if !defined(_WIN32)
    // take only the mapped bytes into out_buffer to unmap the page asap
    out_buffer.assign(mapped_data, mapped_data + out_result.bytes_read);
    ::munmap(map_ptr, map_size);
    const char* data_ptr = out_buffer.data();
#else
    const char* data_ptr = mapped_data;
#endif

    // use memchr to parse lines as this can use SIMD
    const char* current = data_ptr;
    const char* end = data_ptr + out_result.bytes_read;

    while (current < end && out_result.lines.size() < m_max_lines) {
        const std::size_t remaining = static_cast<std::size_t>(end - current);
        const void* next_newline = std::memchr(current, '\n', remaining);

        if (next_newline != nullptr) {
            const char* nl = static_cast<const char*>(next_newline);
            std::size_t line_len = static_cast<std::size_t>(nl - current);

            // remove windows return char (\r) if present
            if (line_len > 0 && current[line_len - 1] == '\r') {
                --line_len;
            }

            out_result.lines.emplace_back(current, line_len);
            current = nl + 1; // Advance past '\n'
        } else {
            // Last line without a trailing newline
            std::size_t line_len = remaining;
            if (line_len > 0 && current[line_len - 1] == '\r') {
                --line_len;
            }
            out_result.lines.emplace_back(current, line_len);
            break;
        }
    }

    return true;
}

} // namespace arex