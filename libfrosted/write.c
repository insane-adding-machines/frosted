/*
 * Frosted version of write.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(int file, const void *ptr, int len);

int write (int file, const void *ptr, int len)
{
    return __syscall__[SYS_WRITE](file, ptr, len);
}
