#include "frosted.h"
#include "locks.h"

static mutex_t *picotcp_lock = NULL;
void pico_lock_init(void)
{
    if (!picotcp_lock)
        picotcp_lock = mutex_init();
}

void pico_lock(void)
{
    if (picotcp_lock)
        mutex_lock(picotcp_lock);
}

int pico_trylock(void)
{
    if(picotcp_lock)
        return suspend_on_mutex_lock(picotcp_lock);
    else
        return 0;
}

int pico_trylock_kernel(void)
{
    if (!picotcp_lock)
        return -EAGAIN;
    return mutex_trylock(picotcp_lock);
}

void pico_unlock(void)
{
    if (picotcp_lock) {
        mutex_unlock(picotcp_lock);
    }
}
