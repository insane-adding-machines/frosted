/* 
 * flibc:
 * Frosted libc implementation for use in the kernel
 *
 * strcmp implementation
 *
 */

#include <string.h>

int strcmp(const char *s1, const char *s2)
{
    int diff = 0;
    while(!diff && *s1)
    {
        diff = (int)*s1 - (int)*s2;
        s1++;
        s2++;
    }
	return diff;
}
