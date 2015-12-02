/*
 * Frosted version of ttyname.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int file, void *ptr, int len);

int ttyname_r(int file, void *ptr, int len)
{
    return __syscall__[SYS_TTYNAME_R](file, ptr, len);
}

static char ttyname_retval[20];
char* ttyname(int file)
{
    if (__syscall__[SYS_TTYNAME_R](file, ttyname_retval, 20) == 0)
        return ttyname_retval;
    return (char *)0;
}
