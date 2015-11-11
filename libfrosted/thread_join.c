/*
 * Frosted version of thread_join.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int pid, uint32_t timeout);

int thread_join(int pid, uint32_t timeout)
{
    return __syscall__[SYS_THREAD_JOIN](pid, timeout);
}

