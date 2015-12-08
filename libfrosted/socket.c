/*
 * Frosted version of socket.
 */

#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;
extern int sys_socket(int, int, int);

int socket(int domain, int type, int proto)
{
    return sys_socket(domain, type, proto);
}
