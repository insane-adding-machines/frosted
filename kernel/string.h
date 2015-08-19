#ifndef FLIBC_STRING_H
#define FLIBC_STRING_H

#include <stddef.h>

extern void * memset(void *s, int c, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
size_t strlen(const char *s);


#endif /* FLIBC_STRING_H */
