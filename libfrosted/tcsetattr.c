/*
 * Stub version of tcsetattr.
 */

#include "frosted_api.h"
#include <errno.h>
#include <termios.h>
#undef errno
extern int errno;

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
  errno = ENOSYS;
  return -1;
}

