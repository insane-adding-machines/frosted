/*
 * Frosted version of semaphore.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_sem_post(sem_t *s);
extern int sys_sem_wait(sem_t *s);
extern int sys_sem_destroy(sem_t *s);

int sem_post(sem_t *s)
{
    return sys_sem_post(s);
}

int sem_wait(sem_t *s)
{
    return sys_sem_wait(s);
}

int sem_destroy(sem_t *s)
{
    return sys_sem_destroy(s);
}
