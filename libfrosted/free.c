/*
 * Frosted version of free.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void sys_free(void * ptr);

void free(void * ptr)
{
    sys_free(ptr);
}

