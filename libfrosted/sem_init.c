/*
 * Frosted version of sem_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern sem_t *sys_sem_init(int val);

sem_t *sem_init(int val)
{
    return sys_sem_init(val);
}

