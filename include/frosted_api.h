#include <stdlib.h>
#include <stdint.h>
/* Syscalls API for frosted. 
 * Any Application may link to the OS using these system calls.
 *
 */

#ifndef INC_FROSTED_API
#define INC_FROSTED_API
int setclock(unsigned int ms);
int sleep(unsigned int ms);
int suspend(unsigned int ms);
int thread_create(void (*init)(void *), void *arg, unsigned int prio);
int getpid(void);
int getppid(void);
int open(const char *path, int mode);
int close(int fd);
unsigned int gettimeofday(unsigned int *ms);
#endif
