/*
 * Frosted version of stat.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(char * file, struct stat *st);


int stat(const char *file, struct stat *st)
{
    return __syscall__[SYS_STAT](file, st);
}

/* TODO: stub - lstat should stat symbolic links themselves, instead of following them */
int lstat(const char *file, struct stat *st)
{
    return __syscall__[SYS_STAT](file, st);
}
