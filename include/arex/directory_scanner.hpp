#ifndef AREX_DIRECTORY_SCANNER_HPP
#define AREX_DIRECTORY_SCANNER_HPP

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace arex {

enum class FileType : std::uint8_t {
    Unknown = 0,
    Regular,
    Directory,
    Symlink,
    Other
};

struct DirectoryItem {
    std::string name;
    FileType type{FileType::Unknown};
};

class DirectoryScanner {
public:
    explicit DirectoryScanner(std::size_t initial_item_capacity = 256);
    ~DirectoryScanner() = default;

    DirectoryScanner(const DirectoryScanner&) = delete;
    DirectoryScanner& operator=(const DirectoryScanner&) = delete;

    DirectoryScanner(DirectoryScanner&&) noexcept = default;
    DirectoryScanner& operator=(DirectoryScanner&&) noexcept = default;

    [[nodiscard]] bool scan(const std::filesystem::path& path, std::vector<DirectoryItem>& out_items);
};

} // namespace arex

#endif // AREX_DIRECTORY_SCANNER_HPP