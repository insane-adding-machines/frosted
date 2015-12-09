/*
 * Frosted version of malloc.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void* sys_malloc(int size);

void * malloc(int size)
{
    return sys_malloc(size);
}
