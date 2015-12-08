/*
 * Frosted version of poll.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;

struct pollfd;

extern int sys_poll(struct pollfd *pfd, int nfds, int timeout);

int poll(struct pollfd *pfd, int nfds, int timeout)
{
    return sys_poll(pfd, nfds, timeout);
}
