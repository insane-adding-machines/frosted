/*
* Frosted version of shutdown
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_shutdown(int sd, int how);


int shutdown(int sd, int how)
{
    return sys_shutdown(sd, how);
}
