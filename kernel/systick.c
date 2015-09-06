#include "frosted.h"
#include "heap.h"
volatile unsigned int jiffies = 0u;
volatile unsigned int _n_int = 0u;
int _clock_interval = 1;
static int _sched_active = 0;


void frosted_scheduler_on(void)
{
    irq_set_priority(PendSV_IRQn, 2); 
    irq_set_priority(SVCall_IRQn, 1);
    irq_set_priority(SysTick_IRQn, 0);
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
struct ktimer *ktimer_add(uint32_t count, void (*handler)(uint32_t, void *), void *arg)
{
    struct ktimer *t = kalloc(sizeof(struct ktimer));
    if (!t)
        return NULL;
    t->expire_time = jiffies + count;
    t->handler = handler;
    t->arg = arg;
    heap_insert(ktimer_list, t);
    return t;
}

/* Disable existing timer */
void ktimer_cancel(struct ktimer *t)
{
    if (t)
        t->handler = NULL;
}

/* Check expired timers */
static void ktimers_check(void)
{
    struct ktimer *t; 
    struct ktimer t_previous;
    if (!ktimer_list)
        return;
    t = heap_first(ktimer_list);
    while ((t) && (t->expire_time < jiffies)) {
        if (t->handler) {
            t->handler(jiffies, t->arg);
        }
        heap_peek(ktimer_list, &t_previous); 
        kfree(t);
        t = heap_first(ktimer_list);
    }
}


void SysTick_Handler(void)
{
    //irq_off(); // XXX Fixme
    SysTick_Hook();
    jiffies+= _clock_interval;
    _n_int++;

    ktimers_check();

    if (_sched_active && ((task_timeslice() == 0) || (!task_running()))) {
        schedule();
    }
    //irq_on();
}

void SysTick_on(void)
{
    int clock;
    clock = (SystemCoreClock * _clock_interval) / 1000;
    SystemCoreClockUpdate();
    SysTick_Config(clock);
}

void SysTick_off(void)
{
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_ENABLE_Msk;                    /* Disable SysTick IRQ  */
}

int SysTick_interval(unsigned long interval)
{
    _clock_interval = interval;
    SysTick_on();
}





