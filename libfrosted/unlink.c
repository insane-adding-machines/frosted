/*
 * Frosted version of unlink.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (*__syscall__[])(char *name);

int unlink (char * name)
{
    return __syscall__[SYS_UNLINK](name);
}
