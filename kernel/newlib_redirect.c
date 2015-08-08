/* 
 * This will redirect newlib syscalls
 *      malloc(), calloc(), realloc(), free(), sbrk()
 *      to the Frosted heap allocator
 *
 */

#include "malloc.h"
#include "stdint.h"

void * _sbrk(int incr)
{
   extern char   end;           /* Set by linker */
   static char * heap_end;
   char *        prev_heap_end;

   if (heap_end == 0)
     heap_end = &end;

   prev_heap_end = heap_end;
   heap_end += incr;

   return (void *) prev_heap_end;
}

void * _realloc_r(struct _reent *re, void * ptr, size_t size)
{
    (void)re;
    return f_realloc(ptr, size);
}

void * _calloc_r(struct _reent *re, size_t num, size_t size)
{
    (void)re;
    return f_calloc(num, size);
}

void * _malloc_r(struct _reent *re, size_t size)
{
    (void)re;
    return f_malloc(size);
}

void _free_r(struct _reent *re, void * ptr)
{
    (void)re;
    f_free(ptr);
}

void * calloc(size_t num, size_t size)
{
    return f_calloc(num, size);
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

