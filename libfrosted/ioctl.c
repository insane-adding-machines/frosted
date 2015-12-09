/*
 * Frosted version of ioctl.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_ioctl(int fd, const uint32_t cmd, void *arg);

int ioctl (int fd, const uint32_t cmd, void *arg)
{
    return sys_ioctl(fd, cmd, arg);
}
