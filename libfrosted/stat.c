/*
 * Stub version of stat.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;
struct stat;

int stat(const char *file, struct stat *st)
{
  errno = ENOSYS;
  return -1;
}

