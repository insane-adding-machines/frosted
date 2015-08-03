/* 
 * This will redirect newlib's malloc(), calloc(), realloc() and free() functions
 * to the Frosted heap allocator
 *
 * Author: Maxime Vincent
 *
 */

#include "malloc.h"
#include "stdint.h"

void * _realloc_r(struct _reent *re, void * old_addr, size_t new_size)
{
    (void)re;

    uint32_t* len = (uint32_t*)(((uint8_t*)old_addr) - 4);
    uint32_t size;
    void* result = NULL;

    result = f_malloc(new_size);
    size = *len & 0xffff;

    if (size > new_size)
        size = new_size;

    memcpy(result, old_addr, size);
    f_free(old_addr);

    return result;
}

void * _calloc_r(struct _reent *re, size_t num, size_t size)
{
    void * ptr = f_malloc(num*size);

    if (ptr)
        memset(ptr, 0, num*size);

    return ptr;
}

void * _malloc_r(struct _reent *re, size_t size)
{
    return f_malloc(size);
}

void _free_r(struct _reent *re, void * ptr)
{
    f_free(ptr);
}

void * calloc(size_t num, size_t size)
{
    void * ptr = f_malloc(num*size);
    if (ptr)
        memset(ptr, 0, num*size);
    return ptr;
}

void * malloc(size_t size)
{
    return f_malloc(size);
}

#undef free
void free(void * ptr)
{
    f_free(ptr);
}

