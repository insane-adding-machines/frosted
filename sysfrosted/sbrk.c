/* Version of sbrk for Frosted operating system.  */

#include "frosted_api.h"
#include <stddef.h>

void * _sbrk (int incr)
{
    /* No sbrk needed for Frosted Malloc */

    // extern char   end; /* Set by linker.  */
    // static char * heap_end;
    // char *        prev_heap_end;

    // if (heap_end == 0)
    //   heap_end = & end;

    // prev_heap_end = heap_end;
    // heap_end += incr;

    // return (void *) prev_heap_end;
    return NULL;
}
