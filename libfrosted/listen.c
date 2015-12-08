/*
* Frosted version of listen
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_listen(int sd, int backlog);


int listen(int sd, int backlog)
{
    return sys_listen(sd, backlog);
}
