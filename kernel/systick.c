#include "frosted.h"
volatile unsigned int jiffies = 0u;
volatile unsigned int _n_int = 0u;
int _clock_interval = 1;
static int _sched_active = 0;


void frosted_scheduler_on(void)
{
    interrupt_set_priority(PendSV_IRQn, 0); // HIGHEST FOR NOW
    interrupt_set_priority(SVCall_IRQn, 0); //OS_IRQ_MAX_PRIO);
    _sched_active = 1;
}

void frosted_scheduler_off(void)
{
    _sched_active = 0;
}

void __attribute__((weak)) SysTick_Hook(void)
{
}

void SysTick_Handler(void)
{
    //interrupts_off(); // XXX Fixme
    SysTick_Hook();
    jiffies+= _clock_interval;
    _n_int++;

    if (_sched_active && (task_timeslice() == 0)) {
        schedule();
    }
    //interrupts_on();
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





