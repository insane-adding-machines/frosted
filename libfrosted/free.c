/*
 * Frosted version of free.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void (**__syscall__)(void * ptr);

void free(void * ptr)
{
    __syscall__[SYS_FREE](ptr);
}

