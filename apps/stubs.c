#include <unistd.h>
#include <signal.h>
#include <errno.h>

extern int errno;

int setpgid(pid_t pid, pid_t pgid)
{
    if (pid!=pgid)
        return -1;
    return 0;
}


int tcgetpgrp(int fd)
{
    return getpid();
}

int tcsetpgrp(int fd, pid_t pgrp)
{
    return 0;
}

int getpgrp(void)
{
    return getpid();
}

