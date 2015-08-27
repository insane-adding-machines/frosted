/*
 * Redirect newlib's syscalls (with _underscore prefix) to Frosted's syscalls
 */

#include "frosted_api.h"
#include "syscalls.h"
#include <errno.h>
#include <stdint.h>
#undef errno
extern int errno;

int _close(int fd)
{
    return close(fd);
}
 
int _execve(char *name, char **argv, char **env)
{
    return _execve(name, argv, env);
}

int _fork(void)
{
    return fork();
}

int _fstat(int fd, struct stat *st)
{
    return fstat(fd, st);
}

int _getpid(void)
{
    return getpid();
}

int _isatty(int file)
{
    return isatty(file);
}

int _kill(int pid, int sig)
{
    return kill(pid, sig);
}

int _link(char *existing, char *new)
{
    return link(existing, new);
}

int _lseek (int fd, int offset, int whence)
{
    return lseek(fd, offset, whence);
}

int _open(char *file, int flags, int mode)
{
    return open(file, flags, mode);
}

int _read(int file, char *ptr, int len)
{
    return read(file, ptr, len);
}

void * _sbrk (int incr)
{
    /* No sbrk needed for Frosted Malloc */
    return NULL;
}

int _stat(const char *file, struct stat *st)
{
    return stat(file, st);
}

clock_t _times(struct tms *buf)
{
    return NULL;
}

int _unlink (char * name)
{
    return unlink(name);
}

int _wait(int  *status)
{
    return wait(status);
}

int _write (int file, char *ptr, int len)
{
    return write(file, ptr, len);
}

//void * _realloc_r(struct _reent *re, void * ptr, size_t size)
//{
//    (void)re;
//    return realloc(ptr, size);
//}
//
//void * _calloc_r(struct _reent *re, size_t num, size_t size)
//{
//    (void)re;
//    return calloc(num, size);
//}
//
//void * _malloc_r(struct _reent *re, size_t size)
//{
//    (void)re;
//    return malloc(size);
//}
//
//void _free_r(struct _reent *re, void * ptr)
//{
//    (void)re;
//    free(ptr);
//}

void * _realloc(void * ptr, size_t size)
{
    return realloc(ptr, size);
}

void * _calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void * _malloc(size_t size)
{
    return malloc(size);
}

void _free(void * ptr)
{
    free(ptr);
}

