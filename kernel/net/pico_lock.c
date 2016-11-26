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

void pico_unlock(void)
{
    if (picotcp_lock)
        mutex_unlock(picotcp_lock);
}
