/* 
 * flibc:
 * Frosted libc implementation for use in the kernel
 *
 * memset implementation
 *
 */

#include <stddef.h>

void * memset(void *s, int c, size_t n)
{
	int *d = (int *)s;

	while (n--) {
		*d++ = c;
	}

	return s;
}
