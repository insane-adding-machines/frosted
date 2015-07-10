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




/* System Calls */
#define SYS_START 0x00
int sys_start(void);

#define SYS_STOP 0x01
int sys_stop(void);

#define SYS_SLEEP 0x03
int sys_sleep(unsigned int ms);


#define SYS_SETCLOCK 0x99
int sys_setclock(unsigned int n);

#define SYS_THREAD_CREATE 0xa0
int sys_thread_create(void (*init)(void *), void *arg, int prio);


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

