/*
* Frosted version of tcsendbreak
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_tcsendbreak(int fd, int duration);


int tcsendbreak(int fd, int duration)
{
    return sys_tcsendbreak(fd, duration);
}
