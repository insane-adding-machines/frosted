/*
 * Stub version of waitpid.
 */

#include "frosted_api.h"
#include <errno.h>
#include <sys/wait.h>
#undef errno
extern int errno;

pid_t waitpid(pid_t pid, int *stat_loc, int options)
{
  errno = ENOSYS;
  return -1;
}

