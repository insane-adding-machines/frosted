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

#ifdef CONFIG_DEVGPIO
#include <libopencm3/stm32/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVGPIO
static const struct gpio_addr gpio_addrs[] = { {.port=GPIOG, .pin=GPIO13,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_6_13"},
                                                                            {.port=GPIOG, .pin=GPIO14,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_6_14"},
#ifdef CONFIG_DEVUART
#ifdef CONFIG_USART_1
                                                                            {.port=GPIOA, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                                                                            {.port=GPIOA, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_2
                                                                            {.port=GPIOA, .pin=GPIO2,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                                                                            {.port=GPIOA, .pin=GPIO3,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_3
#endif
#ifdef CONFIG_UART_4
#endif
#ifdef CONFIG_UART_5
#endif
#ifdef CONFIG_USART_6
#endif
#ifdef CONFIG_UART_7
#endif
#ifdef CONFIG_UART_8
#endif
#endif
};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif

#ifdef CONFIG_DEVUART
static const struct uart_addr uart_addrs[] = { 
#ifdef CONFIG_USART_1
            { 
                .devidx = 2,
                .base = USART1, 
                .irq = NVIC_USART1_IRQ, 
                .rcc = RCC_USART1, 
                .baudrate = 115200,
                .stop_bits = USART_STOPBITS_1,
                .data_bits = 8,
                .parity = USART_PARITY_NONE,
                .flow = USART_FLOWCONTROL_NONE,
            },
#endif
#ifdef CONFIG_USART_2
        { 
            .devidx = 2,
            .base = USART2, 
            .irq = NVIC_USART2_IRQ, 
            .rcc = RCC_USART2, 
            .baudrate = 115200,
            .stop_bits = USART_STOPBITS_1,
            .data_bits = 8,
            .parity = USART_PARITY_NONE,
            .flow = USART_FLOWCONTROL_NONE,
        },
#endif
};
#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))
#endif

void machine_init(struct fnode * dev)
{
#       if CONFIG_SYS_CLOCK == 168000000
            rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
            rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_84MHZ]);
#       elif CONFIG_SYS_CLOCK == 48000000
            rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_48MHZ]);
#       else
#error No valid clock speed selected for STM32F429 Discovery
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev, uart_addrs, NUM_UARTS);
#endif
}

