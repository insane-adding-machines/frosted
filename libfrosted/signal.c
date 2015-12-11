#include <stdint.h>
#include <signal.h>

int sigemptyset(sigset_t *set)
{
    if (!set)
        return -1;
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    int i;
    if (!set)
        return -1;
    for (i = 0; i < SIGMAX; i++)
        *set |= (1 << i);
    return 0;
}

int sigaddset(sigset_t *set, int signum)
{
    if (!set)
        return -1;
    *set |= (1 << signum);
    return 0;
}

int sigdelset(sigset_t *set, int signum)
{
    if (!set)
        return -1;
    *set &= ~(1 << signum);
    return 0;
}

int sigismember(const sigset_t *set, int signum)
{
    if (!set)
        return -1;
    return (*set & (1 << signum))?(1):(0);
}

int sigisemptyset(const sigset_t *set)
{
    if (!set)
        return -1;
    if (*set == 0)
        return 1;
    return 0;
}

int sigorset(sigset_t *dest, const sigset_t *left, const sigset_t *right)
{
    if (!dest || !left || !right)
        return -1;
    *dest = *left | *right;
    return 0;
}

int sigandset(sigset_t *dest, const sigset_t *left, const sigset_t *right)
{
    if (!dest || !left || !right)
        return -1;
    *dest = *left & *right;
    return 0;
}


