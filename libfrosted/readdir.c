/*
 * Frosted version of readdir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(DIR *d, struct dirent *ep);

int readdir(DIR *d, struct dirent *ep)
{
    return __syscall__[SYS_READDIR](d, ep);
}

