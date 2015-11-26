/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the Free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "libopencm3/cm3/systick.h"
#include <libopencm3/lpc17xx/clock.h>
#include <libopencm3/lpc17xx/nvic.h>
#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_DEVGPIO
#include <libopencm3/lpc17xx/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVGPIO
static const struct gpio_addr gpio_addrs[] = {   {.port=GPIO1, .pin=GPIOPIN18, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_18"},
                                                                                {.port=GPIO1, .pin=GPIOPIN20, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_20"},
                                                                                {.port=GPIO1, .pin=GPIOPIN21, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_21"},
                                                                                {.port=GPIO1, .pin=GPIOPIN23, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_23"} 
#ifdef CONFIG_DEVUART
#ifdef CONFIG_UART_0
#endif
#ifdef CONFIG_UART_1
#endif
#ifdef CONFIG_UART_2
#endif
#ifdef CONFIG_UART_3
#endif
#endif
};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif

#ifdef CONFIG_DEVUART
static const struct uart_addr uart_addrs[] = { 
#ifdef CONFIG_UART_0
        { .base = UART0_BASE, .irq = NVIC_UART0_IRQ, },
#endif
#ifdef CONFIG_UART_1
        { .base = UART1_BASE, .irq = NVIC_UART1_IRQ, },
#endif
#ifdef CONFIG_UART_2
        { .base = UART2_BASE, .irq = NVIC_UART2_IRQ, },
#endif
#ifdef CONFIG_UART_3
        { .base = UART3_BASE, .irq = NVIC_UART3_IRQ, },
#endif
};

#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))

static void uart_init(struct fnode * dev)
{
    int i;
    struct module * devuart = devuart_init(dev);

    for (i = 0; i < NUM_UARTS; i++) 
        uart_fno_init(dev, i, &uart_addrs[i]);

    register_module(devuart);
}
#endif

void machine_init(struct fnode * dev)
{

#if CONFIG_SYS_CLOCK == 100000000
     clock_setup(&clock_scale[CLOCK_96MHZ]);
#else
#error No valid clock speed selected for lpc1768mbed
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev);
#endif

}

