/*
 * Frosted version of ioctl.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, const uint32_t cmd, void *arg);

int ioctl (int fd, const uint32_t cmd, void *arg)
{
    return __syscall__[SYS_IOCTL](fd, cmd, arg);
}
