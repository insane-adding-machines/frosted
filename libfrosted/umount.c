/*
 * Frosted version of umount.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_umount(char *target, uint32_t flags);

int umount(char *target, uint32_t flags)
{
    (void)flags; /* flags unimplemented for now */
    return sys_umount(target, flags);
}
