/*
 * Frosted version of mkdir.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include "sys/types.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(const char *path, mode_t mode);

/* TODO: add mode */
int mkdir(const char *path, mode_t mode)
{
    return __syscall__[SYS_MKDIR](path, mode);
}

