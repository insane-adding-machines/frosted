/*
 * Frosted version of sleep.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_sleep(int s);

int sleep(int s)
{
    return sys_sleep(s);
}

