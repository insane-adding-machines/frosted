/*
 * Frosted version of thread_create.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_thread_create(void (*)(void *), void *, unsigned int);

int thread_create(void (*init)(void *), void *arg, unsigned int prio)
{
    return sys_thread_create(init, arg, prio);
}

