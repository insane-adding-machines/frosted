#ifndef FLIBC_STRING_H
#define FLIBC_STRING_H

#include <stddef.h>

extern void * memset(void *s, int c, size_t n);
extern void *memcpy(void *dst, const void *src, size_t n);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t n);
extern size_t strlen(const char *s);


#endif /* FLIBC_STRING_H */
