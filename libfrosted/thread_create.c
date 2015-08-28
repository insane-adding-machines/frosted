/*
 * Frosted version of thread_create.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(void (*)(void *), void *, unsigned int);

int thread_create(void (*init)(void *), void *arg, unsigned int prio)
{
    return __syscall__[SYS_THREAD_CREATE](init, arg, prio);
}

