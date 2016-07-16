/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "locks.h"


/* Semaphore: internal functions */
static void _add_listener(sem_t *s)
{
    int i;
    int pid = scheduler_get_cur_pid();
    for (i = 0; i < s->listeners; i++) {
        if (s->listener[i] == pid)
            return;
        if (s->listener[i] == -1) {
            s->listener[i] = pid;
        }
    }
}

static void _del_listener(sem_t *s)
{
    int i;
    int pid = scheduler_get_cur_pid();
    for (i = 0; i < s->listeners; i++) {
        if (s->listener[i] == pid) {
            s->listener[i] = -1;
            return;
        }
    }
}

static int sem_spinwait(sem_t *s)
{
    if (!s)
        return -EINVAL;
    while (_sem_wait(s) != 0) {
        /* spin ... */
    }
    return 0;
}

/* Semaphore: API */

int sem_trywait(sem_t *s)
{
    if (!s)
        return -EINVAL;
    if(_sem_wait(s) != 0)
        return -EAGAIN;
    return 0;
}

int sem_wait(sem_t *s)
{
    if (scheduler_get_cur_pid() == 0)
        return sem_spinwait(s);
    if (!s)
        return -EINVAL;
    if(_sem_wait(s) != 0) {
        _add_listener(s);
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    _del_listener(s);
    return 0;
}


int sem_post(sem_t *s)
{
    if (!s)
        return -EINVAL;
    if (_sem_post(s) > 0) {
        int i;
        for(i = 0; i < s->listeners; i++) {
            int pid = s->listener[i];
            if (pid > 0) {
                task_resume_lock(pid);
            }
        }
    }
    return 0;
}

int sem_destroy(sem_t *sem)
{
    if (sem->listener)
        kfree(sem->listener);
    kfree(sem);
    return 0;
}

sem_t *sem_init(int val)
{
    sem_t *s = kalloc(sizeof(sem_t));
    if (s) {
        int i;
        s->value = val;
        s->listeners = 8;
        s->listener = kalloc(sizeof(int) * (s->listeners + 1));
        for (i = 0; i < s->listeners; i++)
            s->listener[i] = -1;

    }
    return s;
}

/* Semaphore: Syscalls */
int sys_sem_init_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return (int)sem_init(arg1);
}

int sys_sem_post_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return sem_post((sem_t *)arg1);
}

int sys_sem_wait_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return sem_wait((sem_t *)arg1);
}

int sys_sem_destroy_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return sem_destroy((sem_t *)arg1);
}

/* Mutex: API */
mutex_t *mutex_init()
{
    mutex_t *s = kalloc(sizeof(mutex_t));
    if (s) {
        int i;
        s->value = 1; /* Unlocked. */
        s->listeners = 8;
        s->listener = kalloc(sizeof(int) * (s->listeners + 1));
        for (i = 0; i < s->listeners; i++)
            s->listener[i] = -1;
    }
    return s;
}

void mutex_destroy(mutex_t *s)
{
    if (s->listener)
        kfree(s->listener);
    kfree(s);
}

static int mutex_spinlock(mutex_t *s)
{
    if (!s)
        return -EINVAL;
    while (_mutex_lock(s) != 0) {
        /* spin... */
    }
    return 0;
}

int mutex_trylock(mutex_t *s)
{
    if (!s)
        return -EINVAL;
    if(_mutex_lock(s) != 0)
        return -EAGAIN;
    return 0;
}

int mutex_lock(mutex_t *s)
{
    if (scheduler_get_cur_pid() == 0)
        return mutex_spinlock(s);
    if (!s)
        return -EINVAL;
    if(_mutex_lock(s) != 0) {
        _add_listener(s);
        task_suspend();
        return SYS_CALL_AGAIN;
    }
    _del_listener(s);
    return 0;
}

int mutex_unlock(mutex_t *s)
{
    if (!s)
        return -EINVAL;
    if (_mutex_unlock(s) == 0) {
        int i;
        for(i = 0; i < s->listeners; i++) {
            int pid = s->listener[i];
            if (pid > 0) {
                task_resume_lock(pid);
            }
        }
        return 0;
    }
    return -EAGAIN;
}


/* Mutex: Syscalls */
int sys_mutex_init_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return (int)mutex_init(arg1);
}

int sys_mutex_lock_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return mutex_lock((mutex_t *)arg1);
}

int sys_mutex_unlock_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return mutex_unlock((mutex_t *)arg1);
}

int sys_mutex_destroy_hdlr(int arg1, int arg2, int arg3, int arg4, int arg5)
{
    if (task_ptr_valid(arg1))
        return -EACCES;
    return sem_destroy((sem_t *)arg1); /* Same as semaphore */
}

