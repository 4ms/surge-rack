// POSIX calls that ghc::filesystem uses but the MetaModule firmware does not provide.

#include <errno.h>
#include <sys/stat.h>

int mkdir(const char *path, mode_t mode)
{
    (void)path;
    (void)mode;
    errno = ENOSYS;
    return -1;
}
