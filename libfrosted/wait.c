/*
 * Stub version of wait.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int wait(int  *status)
{
  errno = sys_waitpid(-1, status, 0);
  return -1;
}

