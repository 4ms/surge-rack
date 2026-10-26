#pragma once

// Force-included when building ghc::filesystem (see cmake/surge-engine.cmake).

#include "asset_dirs.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

// The Surge data in the plugin's assets is listed from an index (see asset_dirs.h). Other paths
// go to the firmware (or in the simulator, the host).
static inline int mm_asset_stat(const char *path, struct stat *buf)
{
    int is_dir;
    unsigned size;
    if (!surge_mm_asset_stat(path, &is_dir, &size))
        return 0;
    memset(buf, 0, sizeof(*buf));
    buf->st_mode = is_dir ? (S_IFDIR | 0555) : (S_IFREG | 0444);
    buf->st_size = size;
    return 1;
}

static inline int mm_stat(const char *path, struct stat *buf)
{
    if (mm_asset_stat(path, buf))
        return 0;
#ifdef SURGE_MM_HARDWARE
    // The firmware's stat() sets errno = ENOENT on failure, but that is the firmware's errno:
    // the plugin links its own libc, which has a separate errno. ghc::filesystem only treats
    // a failed stat() as "not found" when errno is ENOENT (or ENOTDIR), and otherwise throws
    // filesystem_error, so fs::exists() on a missing file would throw. Set the plugin's errno.
    if (stat(path, buf) == 0)
        return 0;
    errno = ENOENT;
    return -1;
#else
    return stat(path, buf);
#endif
}

static inline int mm_lstat(const char *path, struct stat *buf)
{
#ifdef SURGE_MM_HARDWARE
    // newlib does not declare lstat(), and the MetaModule has no symlinks.
    return mm_stat(path, buf);
#else
    if (mm_asset_stat(path, buf))
        return 0;
    return lstat(path, buf);
#endif
}

static inline DIR *mm_opendir(const char *path)
{
    if (void *dir = surge_mm_asset_opendir(path, sizeof(struct dirent)))
        return (DIR *)dir;
    return opendir(path);
}

static inline struct dirent *mm_readdir(DIR *dir)
{
    const char *name;
    int is_dir;
    void *storage;
    if (!surge_mm_asset_readdir(dir, &name, &is_dir, &storage))
        return readdir(dir);
    if (!name)
        return NULL;

    struct dirent *entry = (struct dirent *)storage;
    memset(entry, 0, sizeof(*entry));
    strncpy(entry->d_name, name, sizeof(entry->d_name) - 1);
#ifdef DT_DIR
    entry->d_type = is_dir ? DT_DIR : DT_REG;
#endif
    return entry;
}

static inline int mm_closedir(DIR *dir)
{
    if (surge_mm_asset_closedir(dir))
        return 0;
    return closedir(dir);
}

#define stat(path, buf) mm_stat(path, buf)
#define lstat(path, buf) mm_lstat(path, buf)
#define opendir(path) mm_opendir(path)
#define readdir(dir) mm_readdir(dir)
#define closedir(dir) mm_closedir(dir)
