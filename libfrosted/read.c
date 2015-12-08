/*
 * Frosted version of read.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_read(int file, void *ptr, int len);

int read(int file, void *ptr, int len)
{
    return sys_read(file, ptr, len);
}
