/*
* Frosted version of tcgetattr
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_tcgetattr(int fd, struct termios *termios_p);


int tcgetattr(int fd, struct termios *termios_p)
{
    return sys_tcgetattr(fd, termios_p);
}
