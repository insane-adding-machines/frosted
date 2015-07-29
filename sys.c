#include "frosted.h"
#include "syscall_table.h"

static void *sys_syscall_handlers[_SYSCALLS_NR] = {

};

int sys_register_handler(uint32_t n, int(*_sys_c)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5))
{
    if (n >= _SYSCALLS_NR)
        return -1; /* Attempting to register non-existing syscall */

    if (sys_syscall_handlers[n] != NULL)
        return -1; /* Syscall already registered */

    sys_syscall_handlers[n] = _sys_c;
    return 0;
} 

int sys_setclock_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return SysTick_interval(arg1);
}

int sys_start_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    frosted_scheduler_on();
    return 0;
}

int sys_stop_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    frosted_scheduler_off();
    return 0;
}

int sys_sleep_hdlr(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
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
    unsigned int prio = (unsigned int) arg2;

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

int __attribute__((signal,naked)) SVC_Handler(uint32_t n, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    asm volatile (" push {r0-r3, r7, r12, lr}\n");

    int retval;
    int (*call)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5) = NULL;

    if (n >= _SYSCALLS_NR)
        return -1;
    if (sys_syscall_handlers[n] == NULL)
        return -1;

    call = sys_syscall_handlers[n];
    retval = call(arg1, arg2, arg3, arg4, arg5);
    asm volatile ( "mov r9, r0"); // save return value (r0) in r9
    asm volatile (" pop {r0-r3, r7, r12, lr}");
    asm volatile ( "bx lr");
}
