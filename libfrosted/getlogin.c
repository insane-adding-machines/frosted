
/*
* Frosted stub for getlogin
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;

static char * username = "root";

char *getlogin(void)
{
    return username;
}
