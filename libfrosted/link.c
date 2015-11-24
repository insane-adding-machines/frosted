/*
 * Frosted version of link.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(char *src, char *dst);

int link(char *src, char *dst)
{
    return __syscall__[SYS_LINK](src, dst);
}
