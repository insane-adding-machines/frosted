/*
 * Stub version of isatty.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _isatty(int file)
{
  errno = ENOSYS;
  return 0;
}

