#ifndef INC_FROSTED_API
#define INC_FROSTED_API
#include <stdint.h>

typedef unsigned long size_t;
#define NULL 0

static inline void memset(void *ptr, uint8_t byte, size_t count)
{
    register int i;
    uint8_t *array = (uint8_t *)ptr;
    for (i = 0; i < count; i++)
    {
        array[i] = byte;
    }
}


/* Constants */

#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   (O_RDONLY | O_WRONLY)
#define O_CREAT  0x04
#define O_EXCL   0x08
#define O_TRUNC  0x10
#define O_APPEND 0x20

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Syscalls API for frosted. 
 * Any Application may link to the OS using these system calls.
 *
 */
#ifndef KERNEL
int sys_setclock(unsigned int ms);
int sys_sleep(unsigned int ms);
int sys_suspend(unsigned int ms);
int sys_thread_create(void (*init)(void *), void *arg, unsigned int prio);
int sys_mkdir(char *path);
int sys_getpid(void);
int sys_getppid(void);
int sys_open(const char *path, int mode);
int sys_close(int fd);
unsigned int sys_gettimeofday(unsigned int *ms);
#endif /* KERNEL */

#endif
