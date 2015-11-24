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
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/usart.h"
#include "libopencm3/cm3/nvic.h"

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_GPIO_STM32F4
#include <libopencm3/stm32/gpio.h>
#include "gpio.h"

static const struct gpio_addr gpio_addrs[] = { {.port=GPIOD, .rcc=RCC_GPIOD, .pin=GPIO12, .name="gpio_3_12"},
                                                                            {.port=GPIOD, .rcc=RCC_GPIOD, .pin=GPIO13, .name="gpio_3_13"},
                                                                            {.port=GPIOD, .rcc=RCC_GPIOD, .pin=GPIO14, .name="gpio_3_14"},
                                                                            {.port=GPIOD, .rcc=RCC_GPIOD, .pin=GPIO15, .name="gpio_3_15"} };

#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))

/* Common code - to be moved, but where? */
static void gpio_init(struct fnode * dev)
{
    int i;
    struct fnode *node;
    rcc_periph_clock_enable(RCC_GPIOD);

    struct module * devgpio = devgpio_init(dev);

    for(i=0;i<NUM_GPIOS;i++)
    {
        node = fno_create(devgpio, gpio_addrs[i].name, dev);
        if (node)
            node->priv = &gpio_addrs[i];
    }
    register_module(devgpio);
}
#endif

#ifdef CONFIG_DEVUART
                        
static const struct uart_addr uart_addrs[] = { 
        {   
            .base = USART1, 
            .irq = NVIC_USART1_IRQ, 
            .rcc = RCC_USART1, 
            .num_pins = 2,
            .pins[0] = {.port=GPIOA,.rcc=RCC_GPIOA,.pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF7,},
            .pins[1] = {.port=GPIOA,.rcc=RCC_GPIOA,.pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF7,},
        },
        { 
            .base = USART2, 
            .irq = NVIC_USART2_IRQ, 
            .rcc = RCC_USART2, 
            .num_pins = 2,
            .pins[0] = {.port=GPIOA,.rcc=RCC_GPIOA,.pin=GPIO2,.mode=GPIO_MODE_AF,.af=GPIO_AF7,},
            .pins[1] = {.port=GPIOA,.rcc=RCC_GPIOA,.pin=GPIO3,.mode=GPIO_MODE_AF,.af=GPIO_AF7,},
        },
        { 
            .base = USART6, 
            .irq = NVIC_USART6_IRQ, 
            .rcc = RCC_USART6, 
            .num_pins = 2,
            .pins[0] = {.port=GPIOC,.rcc=RCC_GPIOC,.pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF8,},
            .pins[1] = {.port=GPIOC,.rcc=RCC_GPIOC,.pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF8,},
        },
};

#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))

static void uart_init(struct fnode * dev)
{
    int i,j;
    const struct gpio_addr * pcfg;
    struct module * devuart = devuart_init(dev);

    for (i = 0; i < NUM_UARTS; i++) 
    {
        for(j=0;j<uart_addrs[i].num_pins;j++)
        {
            pcfg = &uart_addrs[i].pins[j];
            rcc_periph_clock_enable(pcfg->rcc);
            gpio_mode_setup(pcfg->port, pcfg->mode, GPIO_PUPD_NONE, pcfg->pin);
            if(pcfg->mode == GPIO_MODE_AF)
            {
                gpio_set_af(pcfg->port, pcfg->af, pcfg->pin);
            }
        }
        uart_fno_init(dev, i, &uart_addrs[i]);
        rcc_periph_clock_enable(uart_addrs[i].rcc);
    }
    register_module(devuart);
}
#endif

void machine_init(struct fnode * dev)
{
        /* 401 & 411 run 84 and 100 MHz max respectively */
#       if CONFIG_SYS_CLOCK == 48000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_48MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
        rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_84MHZ]);
#       else
#error No valid clock speed selected for STM32F4x1 Discovery
#endif

#ifdef CONFIG_GPIO_STM32F4
    gpio_init(dev);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev);
#endif

}

