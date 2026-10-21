#pragma once

// ghc::filesystem uses statvfs() only for fs::space(), which Surge does not call.
// The MetaModule has no statvfs(), so this always fails.

#include <errno.h>

struct statvfs
{
    unsigned long f_bsize;
    unsigned long f_frsize;
    unsigned long f_blocks;
    unsigned long f_bfree;
    unsigned long f_bavail;
};

static inline int statvfs(const char *, struct statvfs *)
{
    errno = ENOSYS;
    return -1;
}
