/*
 * Frosted version of unlink.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _unlink (char * name)
{
    return sys_unlink(name);
}

