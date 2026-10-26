#include "asset_dirs.h"
#include "SurgeXT.h"
#include <algorithm>
#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{

struct Entry
{
    const char *path; // relative to build/surge-data
    bool isDir;
    unsigned size;
};

// The entries in build/surge-data, sorted by path (made by metamodule/CMakeLists.txt)
constexpr Entry entries[] = {
#include "surge_data_index.inc"
};

struct AssetDir
{
    std::string path;
    size_t next{0};
    std::string name;
    std::vector<std::max_align_t> entry;
};

std::mutex mutex;
std::vector<AssetDir *> openDirs;

// The path relative to build/surge-data, if it's in there
std::optional<std::string> relativePath(const char *path)
{
    static const std::string root = rack::asset::plugin(pluginInstance, "build/surge-data");

    std::string p;
    for (const char *c = path; *c; ++c)
        if (*c != '/' || p.empty() || p.back() != '/')
            p += *c;
    while (!p.empty() && p.back() == '/')
        p.pop_back();

    if (p == root)
        return "";
    if (p.size() > root.size() && p.starts_with(root) && p[root.size()] == '/')
        return p.substr(root.size() + 1);
    return std::nullopt;
}

// The entry for a path relative to build/surge-data ("" is build/surge-data itself)
std::optional<Entry> find(std::string_view path)
{
    if (path.empty())
        return Entry{"", true, 0};

    auto it = std::lower_bound(std::begin(entries), std::end(entries), path,
                               [](const Entry &e, std::string_view p) { return e.path < p; });
    if (it != std::end(entries) && it->path == path)
        return *it;
    return std::nullopt;
}

std::string_view parent(std::string_view path)
{
    auto slash = path.rfind('/');
    return slash == path.npos ? "" : path.substr(0, slash);
}

AssetDir *asAssetDir(void *dir)
{
    auto *d = static_cast<AssetDir *>(dir);
    return std::find(openDirs.begin(), openDirs.end(), d) != openDirs.end() ? d : nullptr;
}

} // namespace

int surge_mm_asset_stat(const char *path, int *is_dir, unsigned *size)
{
    std::lock_guard lock{mutex};
    auto rel = relativePath(path);
    if (!rel)
        return 0;
    auto e = find(*rel);
    if (!e)
        return 0;

    *is_dir = e->isDir;
    *size = e->size;
    return 1;
}

void *surge_mm_asset_opendir(const char *path, size_t entry_size)
{
    std::lock_guard lock{mutex};
    auto rel = relativePath(path);
    if (!rel)
        return nullptr;
    auto e = find(*rel);
    if (!e || !e->isDir)
        return nullptr;

    auto *d = new AssetDir{*rel};
    d->entry.resize((entry_size + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t));
    openDirs.push_back(d);
    return d;
}

int surge_mm_asset_readdir(void *dir, const char **name, int *is_dir, void **entry)
{
    std::lock_guard lock{mutex};
    auto *d = asAssetDir(dir);
    if (!d)
        return 0;

    *name = nullptr;
    *entry = d->entry.data();
    for (; d->next < std::size(entries); ++d->next)
    {
        std::string_view path = entries[d->next].path;
        if (parent(path) == d->path)
        {
            d->name = d->path.empty() ? path : path.substr(d->path.size() + 1);
            *name = d->name.c_str();
            *is_dir = entries[d->next].isDir;
            ++d->next;
            break;
        }
    }
    return 1;
}

int surge_mm_asset_closedir(void *dir)
{
    std::lock_guard lock{mutex};
    auto *d = asAssetDir(dir);
    if (!d)
        return 0;

    std::erase(openDirs, d);
    delete d;
    return 1;
}
