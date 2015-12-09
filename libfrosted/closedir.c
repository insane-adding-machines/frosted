/*
 * Frosted version of closedir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_closedir(DIR *d);

int closedir(DIR *d)
{
    return sys_closedir(d);
}

