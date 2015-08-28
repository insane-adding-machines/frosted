/*
 * Frosted version of mkdir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(char *path);

int mkdir(char *path)
{
    return __syscall__[SYS_MKDIR](path);
}

