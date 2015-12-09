/*
* Frosted version of getsockopt
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_getsockopt(int sd, int level, int optname, void *optval, unsigned int *optlen);


int getsockopt(int sd, int level, int optname, void *optval, unsigned int *optlen)
{
    return sys_getsockopt(sd, level, optname, optval, optlen);
}
