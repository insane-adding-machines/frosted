/*
 * Frosted version of read.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(int file, char *ptr, int len);

int _read(int file, char *ptr, int len)
{
    return __syscall__[SYS_READ](file, ptr, len);
}

