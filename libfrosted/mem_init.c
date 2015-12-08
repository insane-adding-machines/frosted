/*
 * Frosted version of mem_init.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void sys_mem_init(void * ptr);

void mem_init(void * ptr)
{
    sys_mem_init(ptr);
}

