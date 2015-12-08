/*
 * Frosted version of getcwd.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern char * sys_getcwd(char *, int);

char *getcwd(char *buf, int size)
{
    return sys_getcwd(buf, size);
}
