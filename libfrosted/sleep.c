/*
 * Frosted version of sleep.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int s);

int sleep(int s)
{
    return __syscall__[SYS_SLEEP](s);
}

