#ifndef _FROSTED_MALLOC_H
#define _FROSTED_MALLOC_H

#include "string.h"

#define MEM_KERNEL 0
#define MEM_USER   1
#define MEM_TASK   2
#define MEM_OWNER_MASK 3

struct f_malloc_stats {
    uint32_t malloc_calls;
    uint32_t free_calls;
    uint32_t objects_allocated;
    uint32_t mem_allocated;
};

void * f_malloc(int flags, size_t size);
void * f_calloc(int flags, size_t num, size_t size);
void* f_realloc(int flags, void* ptr, size_t size);
void f_free(void * ptr);

#endif /* _FROSTED_MALLOC_H */
