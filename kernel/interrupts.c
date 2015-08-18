#include "frosted.h"


/* NOTES on ARM CORTEX-M implementation
 * (Cortex-M3, Cortex-M4, Cortex-M4F and Cortex-M7)
 *
 * Cortex-M architecture defines maximum 8 priority bits -> 256 different priorities
 * Different hardware manufacturers often only implement some of these.
 * E.g.:
 *  - TI Stellaris (M3 and M4) implement only 3 prio bits out of 8 -> 8 different priorities
 *  - NXP LPC17xx (M3) implement 5 priority bits out of 8 -> 32 different priorities
 * The devices will always implement the most significant bits. E.g. 0xE0 for Stellaris.
 *
 * Frosted provides 2 different types of interrupts:
 *  - Those masked by OS critical sections (e.g. during system calls; they are allowed to use OS ISR-specific functions)
 *  - Those never masked by the OS. (always on, but cannot use OS functions in the ISR; to be used for maximum performance)
 *
 * ARM Cortex-M pre-empt priority and sub-priority bits:
 *  The implemented priority bits can be further divided into pre-empt prio and sub-prio bits.
 *  * Pre-empt priorities define if an interrupt can pre-empt and already executing interrupt, 
 *    if their pre-emp priority is higher
 *  * Sub-priorities define which of the queued interrupts with the same pre-empt priority will execute first,
 *    if more than one occur at the same time.
 *  It is recommended to assign all bits to be pre-empt priority bits.
 *
 * Any interrupt that will use ISR-safe OS functions from the ISR, must have a _logical_ priority <= OS_IRQ_PRIO
 * This means a _numeric_ priority > OS_IRQ_PRIO.
 *
 * ARM Cortex-M interrupt default to prio 0, which is the highest priority possible.
 * Therefore, you should always manually set the interrupt priority of an IRQ that will use OS functions!
 *
 * NOTE: Frosted uses BASEPRI to mask interrupts, but prio 0 (highest logical priority) cannot be masked by BASEPRI!
 *
 */

// XXX: TODO: Use BASEPRI to mask interrupts, instead of cpsie/cpsid

void interrupt_enable(uint32_t irq_nr)
{
    NVIC_EnableIRQ(irq_nr);
}

void interrupt_set_priority(uint32_t irq_nr, uint32_t prio)
{
    /* prio will be shifted according to NVIC PRIO BITS */
    NVIC_SetPriority(irq_nr, prio);
}

void interrupt_init(void)
{
    NVIC_SetPriorityGrouping(__NVIC_PRIO_BITS); /* assign all bits to be pre-empt prio bits */
}
