/*
* Frosted version of dup
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd);


int dup(int fd)
{
    return __syscall__[SYS_DUP](fd);
}
