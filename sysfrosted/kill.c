/*
 * Frosted version of kill.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(int pid, int  sig);

int _kill(int pid, int sig)
{
    return __syscall__[SYS_KILL](pid, sig);
}


