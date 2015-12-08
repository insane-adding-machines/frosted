/*
 * Frosted version of mutex_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern frosted_mutex_t *sys_mutex_init(void);

frosted_mutex_t *mutex_init(void)
{
    return sys_mutex_init();
}

