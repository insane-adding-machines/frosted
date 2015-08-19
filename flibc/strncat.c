/* 
 * flibc:
 * Frosted libc implementation for use in the kernel
 *
 * strcat implementation
 *
 */

#include <string.h>


char *strncat(char *dest, const char *src, size_t n)
{
    int i = 0;
    int j = strlen(dest);
    for (i = 0; i < strlen(src); i++) {
        if (j >= (n - 1)) {
            break;
        }
        dest[j++] = src[i];
    }
    dest[j] = '\0';
    return dest;
}
