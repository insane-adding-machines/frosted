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
     static const struct gpio_addr gpio_addrs[] = { 
                 {.base=GPIOA, .pin=GPIO4,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_1_4"},
                 {.base=GPIOA, .pin=GPIO13,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_1_13"},
                 {.base=GPIOA, .pin=GPIO14,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_1_14"},
                 {.base=GPIOA, .pin=GPIO15,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_1_15"},
                 {.base=GPIOB, .pin=GPIO3,.mode=GPIO_MODE_INPUT, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .name="gpio_2_3"},
#ifdef CONFIG_DEVUART
#ifdef CONFIG_USART_1
                 {.base=GPIOB, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                 {.base=GPIOB, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_2
                 {.base=GPIOA, .pin=GPIO2,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                 {.base=GPIOA, .pin=GPIO3,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_3
                 {.base=GPIOB, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                 {.base=GPIOB, .pin=GPIO11,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_6
                 {.base=GPIOC, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
                 {.base=GPIOC, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#endif
     };
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif
     
#ifdef CONFIG_DEVUART
     static const struct uart_addr uart_addrs[] = { 
#ifdef CONFIG_USART_1
             {   .devidx = 1,
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
#ifdef CONFIG_USART_3
             { 
                 .devidx = 3,
                 .base = USART3, 
                 .irq = NVIC_USART3_IRQ, 
                 .rcc = RCC_USART3, 
                 .baudrate = 115200,
                 .stop_bits = USART_STOPBITS_1,
                 .data_bits = 8,
                 .parity = USART_PARITY_NONE,
                 .flow = USART_FLOWCONTROL_NONE,
             },
#endif
#ifdef CONFIG_USART_6
             { 
                 .devidx = 6,
                 .base = USART6, 
                 .irq = NVIC_USART6_IRQ, 
                 .rcc = RCC_USART6, 
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
#       if CONFIG_SYS_CLOCK == 48000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_48MHZ]);
#       elif CONFIG_SYS_CLOCK == 84000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
#       elif CONFIG_SYS_CLOCK == 120000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_120MHZ]);
#       elif CONFIG_SYS_CLOCK == 168000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
#       else
#error No valid clock speed selected
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev, uart_addrs, NUM_UARTS);
#endif
}

