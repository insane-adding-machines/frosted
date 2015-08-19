/* 
 * flibc:
 * Frosted libc implementation for use in the kernel
 *
 * strcat implementation
 *
 */

#include <string.h>


char *strcat(char *dest, const char *src)
{
    int i = 0;
    int j = strlen(dest);
    for (i = 0; i < strlen(src); i++) {
        dest[j++] = src[i];
    }
    dest[j] = '\0';
    return dest;
}
