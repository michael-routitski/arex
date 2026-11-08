#include <arex/directory_scanner.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace arex {

namespace {

// Fast helper to convert raw d_type to strongly-typed arex::FileType
inline FileType parse_dirent_type(unsigned char d_type) {
    switch (d_type) {
        case DT_REG:  return FileType::Regular;
        case DT_DIR:  return FileType::Directory;
        case DT_LNK:  return FileType::Symlink;
        case DT_UNKNOWN: return FileType::Unknown;
        default:      return FileType::Other;
    }
}

// Fallback resolver for filesystems where d_type is DT_UNKNOWN (e.g., some NFS/XFS mounts)
inline FileType resolve_stat_fallback(const char* base_path, const char* name) {
    char full_path[4096];
    std::snprintf(full_path, sizeof(full_path), "%s/%s", base_path, name);

    struct stat statbuf{};
    if (::lstat(full_path, &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode))  return FileType::Directory;
        if (S_ISREG(statbuf.st_mode))  return FileType::Regular;
        if (S_ISLNK(statbuf.st_mode))  return FileType::Symlink;
    }
    return FileType::Other;
}

} // namespace

DirectoryScanner::DirectoryScanner(std::size_t initial_item_capacity) {
    (void)initial_item_capacity;
}

bool DirectoryScanner::scan(const std::filesystem::path& path, std::vector<DirectoryItem>& out_items) {
    out_items.clear();

    const std::string path_str = path.string();
    DIR* dir_ptr = ::opendir(path_str.empty() ? "." : path_str.c_str());

    if (!dir_ptr) {
        return false;
    }

    struct dirent* entry = nullptr;

    // do the read directly from kernel buffer without stat calls
    while ((entry = ::readdir(dir_ptr)) != nullptr) {
        const char* name = entry->d_name;

        if (name[0] == '.') {
            if (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')) {
                continue;
            }
        }

        FileType type = parse_dirent_type(entry->d_type);

        // Fallback for filesystems that do not provide d_type in dirent
        if (type == FileType::Unknown) {
            type = resolve_stat_fallback(path_str.c_str(), name);
        }

        out_items.push_back(DirectoryItem{
            .name = std::string(name),
            .type = type
        });
    }

    ::closedir(dir_ptr);

    // Fast inline sort: Directories first (A-Z), then Files (A-Z)
    std::sort(out_items.begin(), out_items.end(), [](const DirectoryItem& a, const DirectoryItem& b) {
        const bool a_is_dir = (a.type == FileType::Directory);
        const bool b_is_dir = (b.type == FileType::Directory);

        if (a_is_dir != b_is_dir) {
            return a_is_dir > b_is_dir; // dirs at the top
        }
        return a.name < b.name;
    });

    return true;
}

} // namespace arex