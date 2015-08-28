/*
 * Frosted version of opendir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern DIR *(**__syscall__)(const char *path);

DIR *opendir(const char *path)
{
    return __syscall__[SYS_OPENDIR](path);
}

