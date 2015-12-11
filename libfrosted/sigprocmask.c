/*
* Frosted version of sigprocmask
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <signal.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
    return sys_sigprocmask(how, set, oldset);
}
