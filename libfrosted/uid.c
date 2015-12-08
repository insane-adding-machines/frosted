/*
* Frosted stubs for unimplemented *uid function family
*/


#include "frosted_api.h"
#include "syscall_table.h"
#include "sys/types.h"
#include <errno.h>
#undef errno
extern int errno;

gid_t getgid(void)
{
    return (gid_t)0;
}

gid_t getegid(void)
{
    return (gid_t)0;
}

uid_t getuid(void)
{
    return (gid_t)0;
}

uid_t geteuid(void)
{
    return (gid_t)0;
}

int issetugid(void)
{
    return (gid_t)0;
}

int seteuid(uid_t euid)
{
    return 0;
}

int setegid(gid_t egid)
{
    return 0;
}

int setgid(gid_t gid)
{
    return 0;
}

int setuid(uid_t euid)
{
    return 0;
}

