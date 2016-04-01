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
int clock_interval = 1;
static int _sched_active = 0;


void frosted_scheduler_on(void)
{
    nvic_set_priority(NVIC_PENDSV_IRQ, 2);
    nvic_set_priority(NVIC_SV_CALL_IRQ, 1);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0);
    nvic_enable_irq(NVIC_SYSTICK_IRQ);
    _sched_active = 1;
    systick_interrupt_enable();
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
static uint32_t ktimers_check(void)
{
    struct ktimer *t; 
    struct ktimer t_previous;
    if (!ktimer_list)
        return -1;
    if (!ktimer_list->n)
        return -1;
    t = heap_first(ktimer_list);
    while ((t) && (t->expire_time < jiffies)) {
        if (t->handler) {
            t->handler(jiffies, t->arg);
        }
        heap_peek(ktimer_list, &t_previous); 
        t = heap_first(ktimer_list);
    }
    return (t->expire_time - jiffies);
}


void sys_tick_handler(void)
{
    uint32_t next_timer = 0;
    volatile uint32_t reload = systick_get_reload();
    uint32_t this_timeslice;
    SysTick_Hook();
    jiffies+= clock_interval;
    _n_int++;
    next_timer = ktimers_check();

#ifdef CONFIG_LOWPOWER
    if (next_timer < 0 || next_timer > 1000){
        next_timer = 1000; /* Wake up every second if timer is too long, or if no timers */
    }

    /* Checking deep sleep */
    if (next_timer >= 1000 && scheduler_can_sleep()) {
        systick_interrupt_disable();
        cputimer_start(next_timer);
        return;
    }
#ifdef CONFIG_TICKLESS
    this_timeslice = task_timeslice();
    if (_sched_active && (this_timeslice == 0) && (!task_running())) {
        schedule();
    } else {
        systick_interrupt_disable();
        cputimer_start(this_timeslice);
    }
    return;
#endif
#endif

    if (_sched_active && ((task_timeslice() == 0) || (!task_running()))) {
        schedule();
        (void)next_timer;
    }
}

