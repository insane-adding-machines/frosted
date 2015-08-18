/* 
 * flibc:
 * Frosted libc implementation for use in the kernel
 *
 * strncmp implementation
 *
 */

#include <string.h>

int strncmp(const char *s1, const char *s2, size_t n)
{
    int diff = 0;
    while(!diff && *s1 && n)
    {
        diff = (int)*s1 - (int)*s2;
        s1++;
        s2++;
        n--;
    }
	return diff;
}
