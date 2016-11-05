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
#include "unicore-mx/cm3/nvic.h"
#include "unicore-mx/cm3/systick.h"
volatile unsigned int jiffies = 0u;
volatile unsigned int _n_int = 0u;
volatile int ktimer_check_pending = 0;
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
    int ret;
    memset(&t, 0, sizeof(t));
    t.expire_time = jiffies + count;
    t.handler = handler;
    t.arg = arg;
    if (!task_in_syscall())
        irq_off();
    ret = heap_insert(ktimer_list, &t);
    if (!task_in_syscall())
        irq_on();
    return ret;
}

/* Delete kernel timer */
int ktimer_del(int tid)
{
    int ret;
    if (tid < 0)
        return -1;
    if (!task_in_syscall())
        irq_off();
    ret = heap_delete(ktimer_list, tid);
    if (!task_in_syscall())
        irq_on();
    return ret;
}

static inline int ktimer_expired(void)
{
    struct ktimer *t;
    return ((ktimer_list) && (ktimer_list->n > 0) &&
            (t = heap_first(ktimer_list)) && (t->expire_time < jiffies));
}

/* Tasklet that checks expired timers */
static void ktimers_check_tasklet(void *arg)
{
    struct ktimer *t;
    struct ktimer t_previous;
    int next_t;

    next_t = -1;

    if ((ktimer_list) && (ktimer_list->n > 0)) {
        irq_off();
        t = heap_first(ktimer_list);
        irq_on();

        while ((t) && (t->expire_time < jiffies)) {
            if (t->handler) {
                t->handler(jiffies, t->arg);
            }
            irq_off();
            heap_peek(ktimer_list, &t_previous);
            t = heap_first(ktimer_list);
            irq_on();
        }
        next_t = (t->expire_time - jiffies);
    }

    ktimer_check_pending = 0;

#ifdef CONFIG_LOWPOWER
    if (next_t < 0 || next_t > 1000) {
        next_t = 1000; /* Wake up every second if timer is too long, or if no
                          timers */
    }

    /* Checking deep sleep */
    if (next_t >= 1000 && scheduler_can_sleep()) {
        systick_interrupt_disable();
        cputimer_start(next_t);
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
}

void sys_tick_handler(void)
{
    uint32_t next_timer = 0;
    volatile uint32_t reload = systick_get_reload();
    uint32_t this_timeslice;
    SysTick_Hook();
    jiffies += clock_interval;
    _n_int++;

    if (ktimer_expired()) {
        if (!ktimer_check_pending) {
            ktimer_check_pending++;
            tasklet_add(ktimers_check_tasklet, NULL);
        }
        task_preempt_all();
        return;
    }

    if (_sched_active && ((task_timeslice() == 0) || (!task_running()))) {
        schedule();
        (void)next_timer;
    }
}
