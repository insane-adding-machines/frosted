/*
 * Frosted version of mutexaphore.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int(**__syscall__)(mutex_t *s);

int mutex_unlock(mutex_t *s)
{
    return __syscall__[SYS_MUTEX_UNLOCK](s);
}

int mutex_lock(mutex_t *s)
{
    return __syscall__[SYS_MUTEX_LOCK](s);
}

int mutex_destroy(mutex_t *s)
{
    return __syscall__[SYS_MUTEX_DESTROY](s);
}
