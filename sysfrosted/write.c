/*
 * Frosted version of write.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _write (int file, char *ptr, int len)
{
    return sys_write(file, ptr, len);
}

