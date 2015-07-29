#include "frosted.h"
volatile unsigned int jiffies = 0u;
volatile unsigned int _n_int = 0u;
int _clock_interval = 1;
static int _sched_active = 0;


void frosted_scheduler_on(void)
{
    NVIC_SetPriority(PendSV_IRQn, 0x80); //XXX: Todo, figure out max prio bits per platform
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
    //irq_off(); // XXX Fixme
    SysTick_Hook();
    jiffies+= _clock_interval;
    _n_int++;

    if (_sched_active) {
        *((uint32_t volatile *)0xE000ED04) = 0x10000000; 
        //pendsv_enable();
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





