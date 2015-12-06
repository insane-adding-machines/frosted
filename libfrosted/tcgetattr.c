/*
* Frosted version of tcgetattr
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, struct termios *termios_p);


int tcgetattr(int fd, struct termios *termios_p)
{
    return __syscall__[SYS_TCGETATTR](fd, termios_p);
}
