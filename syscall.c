#include "frosted.h"

volatile int _syscall_retval = 0;

int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5)
{
    return _syscall(syscall_nr, arg1, arg2, arg3, arg4, arg5);
}

