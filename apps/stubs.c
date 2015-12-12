#include <unistd.h>
#include <signal.h>
#include <errno.h>

extern int errno;

int setpgid(pid_t pid, pid_t pgid)
{
    errno = ENOSYS;
    return -1;
}

int sigaction(int sig, const struct sigaction *restrict act, struct sigaction *restrict oact)
{
    errno = ENOSYS;
    return -1;
}


int tcgetpgrp(int fd)
{
    errno = ENOSYS;
    return -1;
}

int tcsetpgrp(int fd, pid_t pgrp)
{
    errno = ENOSYS;
    return -1;
}

int getpgrp(void)
{
    errno = ENOSYS;
    return -1;
}

