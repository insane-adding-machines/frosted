/*
* Frosted version of dup
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_dup(int fd);


int dup(int fd)
{
    return sys_dup(fd);
}
