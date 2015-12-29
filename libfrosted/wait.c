/*
 * Stub version of wait.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int wait(int  *status)
{
    int ret = sys_waitpid(-1, status, 0);
    if (ret < 0) { 
        errno = 0 - ret;
        return -1;
    }
    return ret;
}

