/*
* Frosted version of sendto
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, const void *buf, unsigned int len, int flags, struct sockaddr_env *se);


int sendto(int sd, const void *buf, unsigned int len, int flags, struct sockaddr *sa, unsigned int socklen)
{
    struct sockaddr_env se;
    int ret;
    se.se_addr = sa;
    se.se_len = socklen;
    return __syscall__[SYS_SENDTO](sd, buf, len, flags, &se);
}
