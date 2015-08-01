#include "frosted.h"
#include "syscall_table.h"


int sys_setclock_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return SysTick_interval(arg1);
}

int sys_suspend_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    Timer_on(arg1);
    return 0;
}

int sys_test_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return (arg1 + (arg2 << 8));
}

int sys_thread_create_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    void (*init)(void *arg);
    void *arg = (void *) arg2;
    unsigned int prio = (unsigned int) arg3;

    init = (void (*)(void *)) arg1;
    return task_create(init, arg, prio);
}

int sys_getpid_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return scheduler_get_cur_pid();
}

int sys_getppid_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return scheduler_get_cur_ppid();
}

