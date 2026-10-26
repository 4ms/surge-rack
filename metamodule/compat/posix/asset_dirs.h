#pragma once

// Lists the Surge data in the plugin's assets (build/surge-data) from an index made at build time
// (see metamodule/CMakeLists.txt), for ghc::filesystem (see ghc_prefix.h).
//
// The plugin's assets are on the MetaModule's RAM disk, where firmware v2.4.0 can't list a
// directory: readdir() returns no entries, and stat() says every path is a regular file. Then
// Surge would find no wavetables (the Wavetable and Window VCOs become Sine VCOs) and no FX presets.
//
// These don't use struct stat or struct dirent, which ghc_prefix.h fills in: the plugin and
// ghc::filesystem don't see the same dirent.h in the simulator (the SDK's and the host's).

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// For a path in the Surge data, sets *is_dir and *size and returns 1. Otherwise returns 0.
int surge_mm_asset_stat(const char *path, int *is_dir, unsigned *size);

// For a directory in the Surge data, returns a handle that lists it, with room for an entry
// (a struct dirent) of entry_size bytes. Otherwise returns NULL.
void *surge_mm_asset_opendir(const char *path, size_t entry_size);

// For a handle from surge_mm_asset_opendir(), sets *name and *is_dir to the next entry's (*name
// is NULL after the last one) and *entry to the handle's room for an entry, and returns 1.
// Otherwise returns 0.
int surge_mm_asset_readdir(void *dir, const char **name, int *is_dir, void **entry);

// For a handle from surge_mm_asset_opendir(), frees it and returns 1. Otherwise returns 0.
int surge_mm_asset_closedir(void *dir);

#ifdef __cplusplus
}
#endif
