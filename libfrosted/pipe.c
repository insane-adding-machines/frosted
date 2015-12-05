/*
* Frosted version of pipe2
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd[2], int flags);


int pipe2(int fd[2], int flags)
{
    return __syscall__[SYS_PIPE2](fd, flags);
}

int pipe(int fd[2])
{
    return __syscall__[SYS_PIPE2](fd, 0);
}
