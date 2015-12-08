/*
 * Frosted version of close.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_close(int fd);

int close(int fd)
{
    return sys_close(fd);
}

