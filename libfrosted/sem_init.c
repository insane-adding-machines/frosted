/*
 * Frosted version of sem_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern sem_t *(**__syscall__)(int val);

sem_t *sem_init(int val)
{
    return __syscall__[SYS_SEM_INIT](val);
}

