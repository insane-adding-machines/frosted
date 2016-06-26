#ifndef _FROSTED_MALLOC_H
#define _FROSTED_MALLOC_H

#include "string.h"
#include "stdint.h"

#define MEM_KERNEL 0
#define MEM_USER   1
#define MEM_TASK   2

#ifdef CONFIG_TCPIP_MEMPOOL
#   define MEM_TCPIP  4
#   define MEM_OWNER_MASK 7
#else
#   define MEM_TCPIP MEM_KERNEL
#   define MEM_OWNER_MASK 3
#endif


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

/* Free up heap of a specific pid */
void f_proc_heap_free(int pid);

/* Get heap usage for specific pid */
uint32_t f_proc_heap_count(int pid);

#endif /* _FROSTED_MALLOC_H */
