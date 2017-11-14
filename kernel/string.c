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
 *      Authors: Daniele Lacamera, Maxime Vincent, brabo
 *
 */

#include <stddef.h>
#include <string.h>

void *memset(void *s, int c, size_t n)
{
	unsigned char *d = (unsigned char *)s;

	while (n--) {
		*d++ = (unsigned char)c;
	}

	return s;
}

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

int strcmp(const char *s1, const char *s2)
{
    int diff = 0;

    while (!diff && *s1) {
        diff = (int)*s1 - (int)*s2;
        s1++;
        s2++;
    }

	return diff;
}

int strcasecmp(const char *s1, const char *s2)
{
    int diff = 0;

    while (!diff && *s1) {
        diff = (int)*s1 - (int)*s2;

        if ((diff == 'A' - 'a') || (diff == 'a' - 'A'))
            diff = 0;

        s1++;
        s2++;
    }

	return diff;
}

size_t strlen(const char *s)
{
    int i = 0;

    while (s[i] != 0)
        i++;

    return i;
}

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

int strncmp(const char *s1, const char *s2, size_t n)
{
    int diff = 0;

    while (n > 0) {
        diff = (unsigned char)*s1 - (unsigned char)*s2;
        if (diff || !*s1)
            break;
        s1++;
        s2++;
        n--;
    }

    return diff;
}

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

    while (!diff && n) {
        diff = (int)*s1 - (int)*s2;
        s1++;
        s2++;
        n--;
    }

	return diff;
}
