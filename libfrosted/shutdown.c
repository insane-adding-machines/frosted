/*
* Frosted version of shutdown
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, int how);


int shutdown(int sd, int how)
{
    return __syscall__[SYS_SHUTDOWN](sd, how);
}
