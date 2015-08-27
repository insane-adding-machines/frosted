/*
 * Frosted version of close.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(int fd);

int _close(int fd)
{
    return __syscall__[SYS_CLOSE](fd);
}

int close(int fd)
{
    return __syscall__[SYS_CLOSE](fd);
}

