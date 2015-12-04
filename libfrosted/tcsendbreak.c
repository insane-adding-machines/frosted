/*
* Frosted version of tcsendbreak
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, int duration);


int tcsendbreak(int fd, int duration)
{
    return __syscall__[SYS_TCSETATTR](fd, duration);
}
