#ifndef AREX_FILE_PREVIEWER_HPP
#define AREX_FILE_PREVIEWER_HPP

#include <cstddef>
#include <filesystem>
#include <string_view>
#include <vector>

namespace arex {

struct PreviewResult {
    std::vector<std::string_view> lines;
    bool is_binary{false};
    bool is_empty{false};
    std::size_t bytes_read{0};
};

class FilePreviewer {
public:
    explicit FilePreviewer(std::size_t max_lines = 16, std::size_t max_bytes = 16384);
    ~FilePreviewer() = default;

    FilePreviewer(const FilePreviewer&) = delete;
    FilePreviewer& operator=(const FilePreviewer&) = delete;

    FilePreviewer(FilePreviewer&&) noexcept = default;
    FilePreviewer& operator=(FilePreviewer&&) noexcept = default;

    // out_buffer must remain alive while using out_result.lines 
    [[nodiscard]] bool preview_file(
        const std::filesystem::path& path,
        std::vector<char>& out_buffer,
        PreviewResult& out_result
    );

private:
    std::size_t m_max_lines{16};
    std::size_t m_max_bytes{16384};
};

} // namespace arex

#endif // AREX_FILE_PREVIEWER_HPP