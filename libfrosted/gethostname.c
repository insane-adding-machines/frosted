
/*
* Frosted stub for gethostname
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include "sys/types.h"
#include "string.h"
#include <errno.h>

#undef errno
extern int errno;

static char * hostname = "frosted";

int gethostname(char *name, size_t namelen)
{
    strncpy(name, hostname, namelen);
    return 0;
}
