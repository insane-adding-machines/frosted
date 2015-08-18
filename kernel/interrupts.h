
#ifndef FROSTED_INTERRUPTS_H
#define FROSTED_INTERRUPTS_H

#include "system.h"

#define OS_LOWEST_IRQ_PRIO      ((1<<__NVIC_PRIO_BITS)-1)       /* 7 on a Stellaris */
#define OS_HALFWAY_IRQ_PRIO     ((1<<(__NVIC_PRIO_BITS-1))-1)   /* 3 on a Stellaris */

#define OS_IRQ_PRIO         OS_LOWEST_IRQ_PRIO     /* Kernel will run at this prio */
#define OS_IRQ_MAX_PRIO     OS_HALFWAY_IRQ_PRIO    /* IRQs using kernel API can run at most at this prio */

#ifdef DEBUG
    static void interrupts_off(void)
    {
    }
    
    static void interrupts_on(void)
    {
    }
#else
    /* Inline kernel utils */

    static inline void interrupt_setmask(void)
    {
        asm volatile ("movs r0, %0 \n" :: "r" (OS_IRQ_MAX_PRIO));
        asm volatile ("msr  basepri, r0         \n");
    }
    
    static inline void interrupt_clearmask(void)
    {
        asm volatile ("movs r0, 0             \n");
        asm volatile ("msr  basepri, r0       \n");
    }

    static inline void interrupts_off(void)
    {
        asm volatile ("cpsid i                \n");
        //interrupt_setmask();
    }
    
    static inline void interrupts_on(void)
    {
        asm volatile ("cpsie i                \n");
        //interrupt_clearmask();
    }
#endif

void interrupt_enable(uint32_t irq_nr);
void interrupt_set_priority(uint32_t irq_nr, uint32_t prio);
void interrupt_init(void);

#endif /* FROSTED_INTERRUPTS_H */
