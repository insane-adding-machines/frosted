/*
* Frosted version of setsockopt
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, int level, int optname, void *optval, unsigned int optlen);


int setsockopt(int sd, int level, int optname, void *optval, unsigned int optlen)
{
    return __syscall__[SYS_SETSOCKOPT](sd, level, optname, optval, optlen);
}
