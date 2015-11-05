#include <stdint.h>

#ifndef FROSTED_HAL_INCLUDE
#define FROSTED_HAL_INCLUDE

#define INT_SYS  0x00010000
#define IRQN_NMI (0x02 | INT_SYS)
#define IRQN_MMI (0x04 | INT_SYS)
#define IRQN_BFI (0x05 | INT_SYS)
#define IRQN_UFI (0x06 | INT_SYS)
#define IRQN_SVC (0x0B | INT_SYS)
#define IRQN_DMI (0x0C | INT_SYS)
#define IRQN_PSV (0x0E | INT_SYS)
#define IRQN_TCK (0x0F | INT_SYS)

/* HAL: CPU abstraction */
void hal_irqprio_config(void);
void hal_irq_set_prio(uint32_t irqn, uint32_t prio);
void hal_irq_on(uint32_t irqn);
void hal_irq_off(uint32_t irqn);
void hal_irq_set_pending(uint32_t irqn);
void hal_irq_clear_pending(uint32_t irqn);
uint32_t hal_irq_get_pending(uint32_t irqn);
uint32_t hal_irq_get_active(uint32_t irqn);
void hal_systick_config(uint32_t ticks);
void hal_systick_stop(void);

/* HAL: Board abstraction */
int hal_board_init(void);
extern const uint32_t NVIC_PRIO_BITS;
extern const uint32_t SYS_CLOCK;

#endif
