/*
 * Frosted version of link.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_link(char *src, char *dst);

int link(char *src, char *dst)
{
    return sys_link(src, dst);
}
