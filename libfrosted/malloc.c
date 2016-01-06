/*
 * Frosted version of malloc.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern void* sys_malloc(int size);
extern void* sys_realloc(void *ptr, int size);

void * malloc(int size)
{
    return sys_malloc(size);
}

void * realloc(void *ptr, int size)
{
    return sys_realloc(ptr, size);
}
