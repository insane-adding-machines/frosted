/*
 * Frosted version of read.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _read(int file, char *ptr, int len)
{
    return sys_read(file, ptr, len);
}

