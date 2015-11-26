#ifndef INC_STM32F4
#define INC_STM32F4

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_GPIO_STM32F4
#include "gpio.h"
#endif

#ifdef CONFIG_GPIO_STM32F4
void gpio_init(struct fnode * dev,  const struct gpio_addr gpio_addrs[], int num_gpios);
#endif

#ifdef CONFIG_DEVUART
void uart_init(struct fnode * dev, const struct uart_addr uart_addrs[], int num_uarts);
#endif

#endif

