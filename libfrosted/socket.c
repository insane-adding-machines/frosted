/*
 * Frosted version of socket.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int (**__syscall__)(int, int, int);

int socket(int domain, int type, int proto)
{
    return __syscall__[SYS_SOCKET](domain, type, proto);
}
