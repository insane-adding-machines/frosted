/*
 * Frosted version of open.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_open(char * file, int flags, int mode);

int open(char *file, int flags, int mode)
{
    (void)flags; /* flags unimplemented for now */
    return sys_open(file, flags, mode);
}
