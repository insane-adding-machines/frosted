/*
* Frosted version of sigsuspend
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <signal.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_sigsuspend(const sigset_t mask);


int sigsuspend(const sigset_t mask)
{
    return sys_sigsuspend(mask);
}
