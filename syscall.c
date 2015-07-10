#include "frosted.h"

volatile int _syscall_retval = 0;

static int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    _syscall(syscall_nr, arg1, arg2, arg3);
    return _syscall_retval;
}

int sys_setclock(unsigned int n)
{
    return syscall(SYS_SETCLOCK, n, 0, 0);
}

int sys_start(void)
{
    return syscall(SYS_START, 0, 0, 0);
}

int sys_sleep(unsigned int ms)
{
    return syscall(SYS_SLEEP, ms, 0, 0);
}

int sys_thread_create(void (*init)(void *), void *arg, int prio)
{
    return syscall(SYS_THREAD_CREATE, init, arg, prio);
}

