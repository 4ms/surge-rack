#pragma once

// Force-included when building ghc::filesystem (see cmake/surge-engine.cmake).

#include <errno.h>
#include <sys/stat.h>

// The firmware's stat() sets errno = ENOENT on failure, but that is the firmware's errno:
// the plugin links its own libc, which has a separate errno. ghc::filesystem only treats
// a failed stat() as "not found" when errno is ENOENT (or ENOTDIR), and otherwise throws
// filesystem_error, so fs::exists() on a missing file would throw. Set the plugin's errno.
static inline int mm_stat(const char *path, struct stat *buf)
{
    if (stat(path, buf) == 0)
        return 0;
    errno = ENOENT;
    return -1;
}
#define stat(path, buf) mm_stat(path, buf)

// newlib does not declare lstat(), and the MetaModule has no symlinks.
static inline int lstat(const char *path, struct stat *buf) { return mm_stat(path, buf); }
