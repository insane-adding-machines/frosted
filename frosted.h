#ifndef FROSTED_INCLUDED_H
#define FROSTED_INCLUDED_H
//#define DEBUG

#include <stdint.h>
#include "chip.h"

/* generics */
volatile unsigned int jiffies;
volatile int _syscall_retval;
void frosted_Init(void);

/* Systick & co. */
int _clock_interval;
void SysTick_Hook(void);
int SysTick_Interval(unsigned long interval);
void SysTick_on(void);
void SysTick_off(void);

/* scheduler */
void pendsv_enable(void);
int syscall(uint32_t syscall_nr, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4, uint32_t arg5);

#ifdef DEBUG
static void irq_off(void)
{
}

static void irq_on(void)
{
}

#else

/* Inline kernel utils */
static __attribute__((always_inline)) void irq_off(void)
{
    __asm("cpsid i");
}

static __attribute__((always_inline)) void irq_on(void)
{
    __asm("cpsie i");
}
#endif

void task_run(void);

#endif /* BSP_INCLUDED_H */

