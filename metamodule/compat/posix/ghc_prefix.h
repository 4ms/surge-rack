#pragma once

// Force-included when building ghc::filesystem (see cmake/surge-engine.cmake).
// newlib does not declare lstat(), and the MetaModule has no symlinks.

#include <sys/stat.h>

static inline int lstat(const char *path, struct stat *buf) { return stat(path, buf); }
