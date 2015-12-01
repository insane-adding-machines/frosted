/*
 * Frosted version of exit.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int val);

int exit(int val)
{
    return __syscall__[SYS_EXIT](val);
}

int _exit(int val) {
    return __syscall__[SYS_EXIT](val);
}


