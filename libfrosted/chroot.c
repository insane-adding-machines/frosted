
/*
* Frosted stub for unimplemented chroot function
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;

int chroot(const char *path)
{
    /* chroot always fails */
    return -1;
}
