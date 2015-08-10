/* NEWLIB CODE
 * sbrk implementation that _does_ check for the stack pointer,
 * such that overallocation will be detected!
 * 
 * Note that the stack pointer can still be incremented to go over
 * the currently allocated heap, and can still result in heap and
 * stack collision.
 */

#include <errno.h>

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");

extern int errno;
char * prev_heap_end;

char * _sbrk (int incr)
{
  extern char end asm ("end"); /* Defined by the linker.  */
  static char * heap_end = NULL;
  //char * prev_heap_end;

  if (heap_end == NULL)
    heap_end = & end;
  
  prev_heap_end = heap_end;
  
  if (heap_end + incr > stack_ptr)
  {
    errno = ENOMEM;
    return (char *) -1;
  }
  
  heap_end += incr;

  return prev_heap_end;
}

