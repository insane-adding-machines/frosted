/*
 * Frosted version of closedir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(DIR *d);

int closedir(DIR *d)
{
    return __syscall__[SYS_CLOSEDIR](d);
}

