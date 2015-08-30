/*
* Frosted version of bind
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, struct sockaddr_env *se);


int bind(int sd, struct sockaddr *sa, unsigned int socklen)
{
    struct sockaddr_env se;
    int ret;
    se.se_addr = sa;
    se.se_len = socklen;
    return __syscall__[SYS_BIND](sd, &se);
}
