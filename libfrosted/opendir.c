/*
 * Frosted version of opendir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern DIR *sys_opendir(const char *path);

DIR *opendir(const char *path)
{
    return sys_opendir(path);
}

