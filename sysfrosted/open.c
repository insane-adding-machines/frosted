/*
 * Frosted version of open.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _open(char *file, int flags, int mode)
{
    (void)flags; /* flags unimplemented for now */
    return sys_open(file, mode);
}

