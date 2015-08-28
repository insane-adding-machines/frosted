/*
 * Frosted version of getppid.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(void);

int getppid(void)
{
    return __syscall__[SYS_GETPPID]();
}
