/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "heap.h"
#include "libopencm3/cm3/nvic.h"
volatile unsigned int jiffies = 0u;
volatile unsigned int _n_int = 0u;
int _clock_interval = 1;
static int _sched_active = 0;


void frosted_scheduler_on(void)
{
    nvic_set_priority(NVIC_PENDSV_IRQ, 2);
    nvic_set_priority(NVIC_SV_CALL_IRQ, 1);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0);
    _sched_active = 1;
}

void frosted_scheduler_off(void)
{
    _sched_active = 0;
}

void __attribute__((weak)) SysTick_Hook(void)
{
}

typedef struct ktimer {
    uint32_t expire_time;
    void *arg;
    void (*handler)(uint32_t time, void *arg);
} ktimer;


DECLARE_HEAP(ktimer, expire_time);
static struct heap_ktimer *ktimer_list = NULL;

/* Init function */
void ktimer_init(void)
{
    ktimer_list = heap_init();
}

/* Add kernel timer */
int ktimer_add(uint32_t count, void (*handler)(uint32_t, void *), void *arg)
{
    struct ktimer t;
    t.expire_time = jiffies + count;
    t.handler = handler;
    t.arg = arg;
    return heap_insert(ktimer_list, &t);
}

/* Check expired timers */
static void ktimers_check(void)
{
    struct ktimer *t; 
    struct ktimer t_previous;
    if (!ktimer_list)
        return;
    if (!ktimer_list->size)
        return;
    t = heap_first(ktimer_list);
    while ((t) && (t->expire_time < jiffies)) {
        if (t->handler) {
            t->handler(jiffies, t->arg);
        }
        heap_peek(ktimer_list, &t_previous); 
        t = heap_first(ktimer_list);
    }
}


void sys_tick_handler(void)
{
    SysTick_Hook();
    jiffies+= _clock_interval;
    _n_int++;

    ktimers_check();

    if (_sched_active && ((task_timeslice() == 0) || (!task_running()))) {
        schedule();
    }
}

void SysTick_on(void)
{
    int clock;
    //clock = (SYS_CLOCK * _clock_interval) / 1000;
    //hal_systick_config(clock);
    /* TODO: Enable systick via opencm3 */
}

void SysTick_off(void)
{
    //hal_systick_stop();
}

int SysTick_interval(unsigned long interval)
{
    _clock_interval = interval;
    SysTick_on();
}





