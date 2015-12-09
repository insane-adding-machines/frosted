/*
* Frosted version of sigaction
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <signal.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);


int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    return sys_sigaction(signum, act, oldact);
}
