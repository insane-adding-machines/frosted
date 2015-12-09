/*
 * Frosted version of isatty.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_isatty(int fd);

int isatty(int fd)
{
    return sys_isatty(fd);
}

