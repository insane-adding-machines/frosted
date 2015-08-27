/*
 * Frosted version of lseek.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _lseek (int fd, int offset, int whence)
{
    return sys_seek(fd, offset, whence);
}

