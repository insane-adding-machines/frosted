/*
 * Stub version of times.
 */

#include "frosted_api.h"
#include <time.h>
#include <errno.h>
#undef errno
extern int errno;
struct tms;

clock_t _times(struct tms *buf)
{
  errno = ENOSYS;
  return -1;
}

