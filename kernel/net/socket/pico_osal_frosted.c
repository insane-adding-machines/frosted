/*
 * pico_osal.h
 *
 *  Created on: February 2016
 *      Author: Daniele Lacamera, Maxime Vincent
 * Description: OS Abstraction Layer between PicoTCP and frosted
 *
 */

/* Frosted kernel include */
#include "frosted.h"


/* PicoTCP includes */
#include "pico_defines.h"
#include "pico_config.h"
#include "pico_osal.h"

#define osal_dbg(...)
//#define osal_dbg(...) printf(__VA_ARGS__)

/*****************************************************************************
 * Public functions
 ****************************************************************************/

struct osal_mutex {
    void * mutex;
    uint8_t idx; /* only to keep track of the amount/idx, no real function .. */
};
static uint8_t mtx_number = 0;

int errno = 0;


/* ============= */
/* == SIGNALS == */
/* ============= */

void * pico_signal_init(void)
{
    struct osal_mutex *signal;
    signal = pico_zalloc(sizeof(struct osal_mutex));
    osal_dbg("mi: %p for %p\n", signal, __builtin_return_address(0));
    if (!signal)
        return NULL;
    signal->mutex= sem_init(1);
    signal->idx = mtx_number++;
    return signal;
}

void pico_signal_deinit(void * signal)
{
    struct osal_mutex * mtx = signal;
    sem_destroy(mtx->mutex);
    pico_free(signal);
}

void pico_signal_wait(void * signal)
{
    sem_wait(signal);
}

int pico_signal_wait_timeout(void * signal, int timeout)
{
    /*TODO*/
    int retval = 0;
    if (retval) {
        return 0; /* Success */
    } else {
        return -1; /* Timeout */
    }
}

void pico_signal_send(void * signal)
{
	if(signal != NULL)
    {
        struct osal_mutex * mtx = signal;
        sem_post(mtx);
    }
}

/* ============= */
/* == MUTEXES == */
/* ============= */


void *pico_mutex_init(void)
{
    struct osal_mutex *mutex;
    mutex = pico_zalloc(sizeof(struct osal_mutex));
    osal_dbg("mi: %p for %p\n", mutex, __builtin_return_address(0));
    if (!mutex)
        return NULL;
    mutex->mutex = frosted_mutex_init();
    mutex->idx = mtx_number++;
    return mutex;
}

void pico_mutex_deinit(void * mutex)
{
    frosted_mutex_destroy(mutex);
}

int pico_mutex_lock_timeout(void * mutex, int timeout)
{
    /*TODO*/
    return 
    -1; /*Timeout*/
}

void pico_mutex_lock(void * mutex)
{
    frosted_mutex_lock(mutex);
}

void pico_mutex_unlock(void * mutex)
{
    frosted_mutex_unlock(mutex);
}

