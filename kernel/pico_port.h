#ifndef PICO_PORT_FROSTED_INCLUDED
#define PICO_PORT_FROSTED_INCLUDED
#define PICO_SUPPORT_MUTEX

#ifdef FROSTED
#include "frosted.h"
#include "string.h"
#include "malloc.h"
#include "kprintf.h"



#define pico_free(x) f_free(x)
#define pico_zalloc(x) f_calloc(MEM_TCPIP, x, 1)

static void *pico_mutex_init(void) {
    return mutex_init();
}

static void pico_mutex_lock(void *m) {
    mutex_lock((mutex_t *)m);
}

static void pico_mutex_unlock(void *m) {
    mutex_unlock((mutex_t *)m);
}

static void pico_mutex_deinit(void *m) {
    mutex_destroy((mutex_t *)m);
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
