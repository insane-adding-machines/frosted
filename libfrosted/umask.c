/*
 * Frosted version of close.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;

int umask(uint32_t mode)
{
    return 0;
}

