
#ifndef FROSTED_INTERRUPTS_H
#define FROSTED_INTERRUPTS_H

extern void __set_BASEPRI(int);

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
        __set_BASEPRI(3);
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

#endif /* FROSTED_INTERRUPTS_H */
