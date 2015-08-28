/*
 * Frosted version of mutex_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern mutex_t *(**__syscall__)(void);

mutex_t *mutex_init(void)
{
    return __syscall__[SYS_MUTEX_INIT]();
}

