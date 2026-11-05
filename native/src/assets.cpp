#include "fate/platform.hpp"
#include <algorithm>
#include <cerrno>
#include <fstream>
#include <stdexcept>
#include <system_error>
#include <fcntl.h>
#include <unistd.h>

namespace fate {
namespace {
std::string lower(std::string value) {
    // Asset names are ASCII. Do not case-fold arbitrary UTF-8 bytes with locale.
    for (auto& c : value) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return value;
}
bool within(const std::filesystem::path& root, const std::filesystem::path& path) {
    auto p = path.begin();
    for (auto r = root.begin(); r != root.end(); ++r, ++p)
        if (p == path.end() || *r != *p) return false;
    return true;
}
}

Assets::Assets(std::filesystem::path root) : root_(std::filesystem::canonical(root)) {
    if (!std::filesystem::is_directory(root_)) throw std::runtime_error("Asset root is not a directory");
}
std::filesystem::path Assets::resolve(std::string name) const {
    std::replace(name.begin(), name.end(), '\\', '/');
    std::filesystem::path relative(name);
    if (relative.is_absolute() || name.find(':') != std::string::npos)
        throw std::runtime_error("Asset path must be relative: " + name);
    auto path = root_;
    for (const auto& part : relative) {
        if (part == "." || part.empty()) continue;
        if (part == "..") throw std::runtime_error("Asset path contains parent traversal: " + name);
        auto next = path / part;
        if (!std::filesystem::exists(next)) {
            bool found = false;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (lower(entry.path().filename().string()) == lower(part.string())) {
                    if (found) throw std::runtime_error("Ambiguous asset path: " + name);
                    next = entry.path();
                    found = true;
                }
            }
            if (!found) throw std::runtime_error("Asset not found: " + name);
        }
        path = std::filesystem::canonical(next);
        if (!within(root_, path)) throw std::runtime_error("Asset symlink escapes root: " + name);
    }
    return path;
}
std::vector<std::uint8_t> Assets::read(const std::string& name) const {
    const auto path = resolve(name);
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) throw std::runtime_error("Cannot open asset: " + path.string());
    const auto length = stream.tellg();
    if (length < 0) throw std::runtime_error("Cannot size asset: " + name);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    stream.seekg(0);
    if (!bytes.empty() && !stream.read(reinterpret_cast<char*>(bytes.data()), bytes.size()))
        throw std::runtime_error("Cannot read asset: " + name);
    return bytes;
}

void write_save(const std::filesystem::path& path, const std::vector<std::uint8_t>& data) {
    const auto parent = path.has_parent_path() ? path.parent_path() : std::filesystem::path(".");
    std::filesystem::create_directories(parent);
    auto pattern = (parent / (path.filename().string() + ".tmp.XXXXXX")).string();
    std::vector<char> temporary(pattern.begin(), pattern.end());
    temporary.push_back('\0');
    int fd = mkstemp(temporary.data());
    if (fd < 0) throw std::system_error(errno, std::generic_category(), "Create save");
    try {
        std::size_t offset = 0;
        while (offset < data.size()) {
            const auto count = ::write(fd, data.data() + offset, data.size() - offset);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) throw std::system_error(count < 0 ? errno : EIO, std::generic_category(), "Write save");
            offset += static_cast<std::size_t>(count);
        }
        if (fsync(fd) < 0) throw std::system_error(errno, std::generic_category(), "Flush save");
        const int close_result = close(fd);
        fd = -1;
        if (close_result < 0) throw std::system_error(errno, std::generic_category(), "Close save");
        std::filesystem::rename(temporary.data(), path);
    } catch (...) {
        if (fd >= 0) close(fd);
        unlink(temporary.data());
        throw;
    }
}
} // namespace fate
