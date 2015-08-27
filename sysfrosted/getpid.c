/*
 * Frosted version of getpid.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(void);

int _getpid(void)
{
    return __syscall__[SYS_GETPID]();
}

