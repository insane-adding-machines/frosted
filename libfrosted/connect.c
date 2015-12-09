/*
* Frosted version of connect
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_connect(int sd, struct sockaddr_env *se);


int connect(int sd, struct sockaddr *sa, unsigned int socklen)
{
    struct sockaddr_env se;
    int ret;
    se.se_addr = sa;
    se.se_len = socklen;
    return sys_connect(sd, &se);
}
