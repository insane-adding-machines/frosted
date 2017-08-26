/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */


/*
 * memset implementation
 *
 */

#include <stddef.h>

void * memset(void *s, int c, size_t n)
{
	unsigned char *d = (unsigned char *)s;

	while (n--) {
		*d++ = (unsigned char)c;
	}

	return s;
}
/*
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
/*
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

int strcasecmp(const char *s1, const char *s2)
{
    int diff = 0;
    while(!diff && *s1)
    {
        diff = (int)*s1 - (int)*s2;
        if ((diff == 'A' - 'a') || (diff == 'a' - 'A'))
            diff = 0;
        s1++;
        s2++;
    }
	return diff;
}


/*
 * strcmp implementation
 *
 */

#include <string.h>

size_t strlen(const char *s)
{
    int i = 0;
    while(s[i] != 0)
        i++;
    return i;
}
/*
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
/*
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
        if (!s1 || !s2)
            break;
        s1++;
        s2++;
        n--;
    }
    if (!diff) {
        if (!(*s1) && !(*s2))
            return 0;
        else if (!(*s1))
            return -1;
        else
            return 1;
    }
	return diff;
}

/*
 * memcpy implementation
 *
 */
void *memcpy(void *dst, const void *src, size_t n)
{
    int i;
    const char *s = (const char *)src;
    char *d = (char *)dst;
    for (i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dst;
}

/*
 * strncpy implementation
 *
 */
char *strncpy(char *dst, const char *src, size_t n)
{
    int i;
    for (i = 0; i < n; i++) {
        dst[i] = src[i];
        if (src[i] == '\0')
            break;
    }
    return dst;
}

/*
 * strcpy implementation
 *
 */
char *strcpy(char *dst, const char *src)
{
    int i = 0;
    while(1 < 2) {
        dst[i] = src[i];
        if (src[i] == '\0')
            break;
        i++;
    }
    return dst;
}

int memcmp(const void *_s1, const void *_s2, size_t n)
{
    int diff = 0;
    const unsigned char *s1 = (const unsigned char *)_s1;
    const unsigned char *s2 = (const unsigned char *)_s2;
    while(!diff && n)
    {
        diff = (int)*s1 - (int)*s2;
        s1++;
        s2++;
        n--;
    }
	return diff;
}
