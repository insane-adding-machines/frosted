#ifndef INC_STM32F4EXTI
#define INC_STM32F4EXTI
#include <unicore-mx/cm3/common.h>
#include <unicore-mx/stm32/f4/exti.h>

int exti_register(uint32_t base, uint16_t pin, uint8_t trigger, void (*isr)(void));
void exti_unregister(int pin);
int exti_enable(int idx, int enable);
void exti_init(void);

#endif
