/*
 * Stub version of fork.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int fork(void)
{
  errno = ENOSYS;
  return -1;
}

