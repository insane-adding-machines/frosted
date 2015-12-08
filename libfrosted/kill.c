/*
 * Frosted version of kill.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_kill(int pid, int  sig);

int kill(int pid, int sig)
{
    return sys_kill(pid, sig);
}
