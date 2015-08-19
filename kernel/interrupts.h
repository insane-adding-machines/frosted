
#ifndef FROSTED_INTERRUPTS_H
#define FROSTED_INTERRUPTS_H

#include "system.h"

#define OS_LOWEST_IRQ_PRIO      ((1<<__NVIC_PRIO_BITS)-1)       /* 7 on a Stellaris */
#define OS_HALFWAY_IRQ_PRIO     ((1<<(__NVIC_PRIO_BITS-1))-1)   /* 3 on a Stellaris */

#define OS_IRQ_PRIO         OS_LOWEST_IRQ_PRIO     /* Kernel will run at this prio */
#define OS_IRQ_MAX_PRIO     OS_HALFWAY_IRQ_PRIO    /* IRQs using kernel API can run at most at this prio */

#ifdef DEBUG
    static void irq_off(void)
    {
    }
    
    static void irq_on(void)
    {
    }

    static void irq_setmask(void)
    {
    }
    
    static void irq_clearmask(void)
    {
    }
#else
    /* Inline kernel utils */

    static inline void irq_setmask(void)
    {
        __set_BASEPRI(OS_IRQ_MAX_PRIO);
    }
    
    static inline void irq_clearmask(void)
    {
        __set_BASEPRI(0u);
    }

    static inline void irq_off(void)
    {
        asm volatile ("cpsid i                \n");
    }
    
    static inline void irq_on(void)
    {
        asm volatile ("cpsie i                \n");
    }
#endif

void irq_enable(uint32_t irq_nr);
void irq_set_priority(uint32_t irq_nr, uint32_t prio);
void irq_init(void);

#endif /* FROSTED_INTERRUPTS_H */
