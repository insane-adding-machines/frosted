/*
 * Frosted version of lseek.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, int offset, int  whence);

int lseek (int fd, int offset, int whence)
{
    return __syscall__[SYS_SEEK](fd, offset, whence);
}
