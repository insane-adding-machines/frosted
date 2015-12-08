/*
 * Frosted version of getpid.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_getpid(void);

int getpid(void)
{
    return sys_getpid();
}
