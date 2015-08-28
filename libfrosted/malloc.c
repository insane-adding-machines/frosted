/*
 * Frosted version of malloc.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void* (**__syscall__)(int size);

void * malloc(int size)
{
    return __syscall__[SYS_MALLOC](size);
}
