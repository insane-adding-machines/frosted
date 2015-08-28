/*
 * Frosted version of semaphore.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int(**__syscall__)(sem_t *s);

int sem_post(sem_t *s)
{
    return __syscall__[SYS_SEM_POST](s);
}

int sem_wait(sem_t *s)
{
    return __syscall__[SYS_SEM_WAIT](s);
}

int sem_destroy(sem_t *s)
{
    return __syscall__[SYS_SEM_DESTROY](s);
}
