/*
* Frosted version of accept
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#include <string.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int sd, struct sockaddr_env *se);


int accept(int sd, struct sockaddr *sa, unsigned int *socklen)
{
    struct sockaddr_env se;
    int ret;
    se.se_addr = sa;
    se.se_len = *socklen;
    ret =  __syscall__[SYS_ACCEPT](sd, &se);
    if (ret > 0) {
        if (*socklen < se.se_len)
            return -1;
        memcpy(sa, se.se_addr, se.se_len); 
        *socklen = se.se_len;
    }
    return ret;
}
