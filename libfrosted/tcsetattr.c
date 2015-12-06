/*
* Frosted version of tcsetattr
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <sys/termios.h>
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int fd, int optional_actions, const struct termios *termios_p);


int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    return __syscall__[SYS_TCSETATTR](fd, optional_actions, termios_p);
}
