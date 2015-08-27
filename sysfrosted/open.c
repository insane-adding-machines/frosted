/*
 * Frosted version of open.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(char * file, int flags, int mode);

int _open(char *file, int flags, int mode)
{
    (void)flags; /* flags unimplemented for now */
    return __syscall__[SYS_OPEN](file, flags, mode);
}

int open(char *file, int flags, int mode)
{
    (void)flags; /* flags unimplemented for now */
    return __syscall__[SYS_OPEN](file, flags, mode);
}
