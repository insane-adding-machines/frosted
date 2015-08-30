/*
* Frosted version of dup2
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, int newfd);


int dup2(int fd, int newfd)
{
    return __syscall__[SYS_DUP2](fd, newfd);
}
