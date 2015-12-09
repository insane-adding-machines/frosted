/*
* Frosted version of tcsetattr
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int sys_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);


int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    return sys_tcsetattr(fd, optional_actions, termios_p);
}
