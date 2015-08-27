/*
 * Frosted version of close.
 */

#include "frosted_api.h"
#include <errno.h>
#undef errno
extern int errno;

int _close(int fd)
{
  return sys_close(fd);
}

