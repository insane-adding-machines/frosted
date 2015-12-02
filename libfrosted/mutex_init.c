/*
 * Frosted version of mutex_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern frosted_mutex_t *(**__syscall__)(void);

frosted_mutex_t *mutex_init(void)
{
    return __syscall__[SYS_MUTEX_INIT]();
}

