/*
 * Frosted version of mutexaphore.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern sys_mutex_unlock(frosted_mutex_t *s);
extern sys_mutex_lock(frosted_mutex_t *s);
extern sys_mutex_destroy(frosted_mutex_t *s);

int mutex_unlock(frosted_mutex_t *s)
{
    return sys_mutex(s);
}

int mutex_lock(frosted_mutex_t *s)
{
    return sys_mutex_lock(s);
}

int mutex_destroy(frosted_mutex_t *s)
{
    return sys_mutex_destroy(s);
}
