#ifndef PICO_PORT_FROSTED_INCLUDED
#define PICO_PORT_FROSTED_INCLUDED
#define PICO_SUPPORT_MUTEX

#ifdef FROSTED
#include "frosted.h"
#include "string.h"
int kprintf(const char *format, ...);


#define pico_free(x) f_free(x)
#define pico_zalloc(x) f_calloc(MEM_KERNEL, x, 1)

static void *pico_mutex_init(void) {
    return frosted_mutex_init();
}

static void pico_mutex_lock(void *m) {
    frosted_mutex_lock((frosted_mutex_t *)m);
}

static void pico_mutex_unlock(void *m) {
    frosted_mutex_unlock((frosted_mutex_t *)m);
}

static void pico_mutex_deinit(void *m) {
    frosted_mutex_destroy((frosted_mutex_t *)m);
}

#define dbg kprintf

static inline pico_time PICO_TIME_MS()
{
    return jiffies;
}

static inline pico_time PICO_TIME()
{
    return jiffies / 1000;
}

static inline void PICO_IDLE(void)
{
}

#else

#error "file pico_port.h included, but no FROSTED define."

#endif

#endif
