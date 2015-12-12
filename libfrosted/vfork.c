#include <sys/types.h>

pid_t vfork(void)
{
    pid_t newpid = sys_vfork();
    if (newpid == getpid())
        return 0;
    return newpid;
}


