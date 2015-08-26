#include "frosted.h"

volatile int _syscall_retval = 0;

int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    int ret;
    if (scheduler_get_cur_pid() == 0)
        return -1; /* Syscalls not allowed from kernel */
    do {
        ret = _syscall(syscall_nr, arg1, arg2, arg3, arg4, arg5);
    } while (ret == SYS_CALL_AGAIN);
    return ret;
}

