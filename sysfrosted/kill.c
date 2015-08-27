/*
 * Frosted version of kill.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _kill(int pid, int sig)
{
    return sys_kill(pid, sig);
}

