
/*
* Frosted stubs for password functions
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include <errno.h>
#undef errno
extern int errno;

struct passwd *getpwnam(const char *name)
{
    errno = ENOENT;
    return NULL;
}

struct passwd *getpwuid(uid_t uid)
{
    errno = ENOENT;
    return NULL;
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    *result = NULL;
    return 0;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    *result = NULL;
    return 0;
}
