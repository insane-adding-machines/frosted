/*
 * Frosted version of mkdir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include "sys/types.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_mkdir(const char *path, mode_t mode);

int mkdir(const char *path, mode_t mode)
{
    return sys_mkdir(path, mode);
}

