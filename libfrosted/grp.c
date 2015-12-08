
/*
* Frosted stubs for group functions
*/

#include "frosted_api.h"
#include "syscall_table.h"
#include "grp.h"
#include <errno.h>
#undef errno
extern int errno;

int getgroups(int size, gid_t list[])
{
    /* always fail for now */
    errno = EPERM;
    return -1;
}
