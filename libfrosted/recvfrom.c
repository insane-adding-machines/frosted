/*
* Frosted version of recvfrom
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#include <string.h>
#undef errno
extern int errno;
extern int sys_recvfrom(int sd, void *buf, unsigned int len, int flags, struct sockaddr_env *se);


int recvfrom(int sd, void *buf, unsigned int len, int flags, struct sockaddr *sa, unsigned int *socklen)
{
    struct sockaddr_env se;
    int ret;
    se.se_addr = sa;
    se.se_len = *socklen;
    ret = sys_recvfrom(sd, buf, len, flags, &se);
    if (ret > 0) {
        if (*socklen < se.se_len)
            return -1;
        memcpy(sa, se.se_addr, se.se_len); 
        *socklen = se.se_len;
    }
    return ret;
}
