/*
 * Frosted version of mem_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void (**__syscall__)(void * ptr);

void mem_init(void * ptr)
{
    __syscall__[SYS_MEM_INIT](ptr);
}

