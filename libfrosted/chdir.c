/*
 * Frosted version of chdir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_chdir(char *path);

int chdir(char *path)
{
    return sys_chdir(path);
}

