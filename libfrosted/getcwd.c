/*
 * Frosted version of getcwd.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern char * (**__syscall__)(char *, int);

char *getcwd(char *buf, int size)
{
    return __syscall__[SYS_GETCWD](buf, size);
}
