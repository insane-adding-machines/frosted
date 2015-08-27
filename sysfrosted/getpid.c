/*
 * Frosted version of getpid.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _getpid(void)
{
    return sys_getpid();
}

