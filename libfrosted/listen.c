/*
* Frosted version of listen
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, int backlog);


int listen(int sd, int backlog)
{
    return __syscall__[SYS_LISTEN](sd, backlog);
}
