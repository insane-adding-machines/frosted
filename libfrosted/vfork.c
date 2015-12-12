#include <sys/types.h>

pid_t vfork(void)
{
    return sys_vfork();
}


