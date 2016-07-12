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
 *      Authors: Daniele Lacamera, Maxime Vincent, brabo
 *
 */
#include "frosted.h"
#include "unicore-mx/cm3/systick.h"
#include <unicore-mx/stm32/rcc.h>
#include "unicore-mx/stm32/usart.h"
#include "unicore-mx/cm3/nvic.h"
#include <unicore-mx/stm32/gpio.h>
#include "gpio.h"
#include "uart.h"




#if CONFIG_SYS_CLOCK == 48000000
#    define STM32_CLOCK RCC_CLOCK_3V3_48MHZ
#elif CONFIG_SYS_CLOCK == 84000000
#    define STM32_CLOCK RCC_CLOCK_3V3_84MHZ
#elif CONFIG_SYS_CLOCK == 120000000
#    define STM32_CLOCK RCC_CLOCK_3V3_120MHZ
#elif CONFIG_SYS_CLOCK == 168000000
#    define STM32_CLOCK RCC_CLOCK_3V3_168MHZ
#else
#   error No valid clock speed selected
#endif

static const struct gpio_config Led0 = {
    .base=GPIOG, 
    .pin=GPIO13,
    .mode=GPIO_MODE_OUTPUT, 
    .optype=GPIO_OTYPE_PP, 
    .name="led0"
};
static const struct gpio_config Led1 = {
    .base=GPIOG, 
    .pin=GPIO14,
    .mode=GPIO_MODE_OUTPUT, 
    .optype=GPIO_OTYPE_PP, 
    .name="led1"
};



static const struct uart_config uart_configs[] = {
#ifdef CONFIG_USART_1
    {
        .devidx = 1,
        .base = USART1,
        .irq = NVIC_USART1_IRQ,
        .rcc = RCC_USART1,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,

        .pio_rx = {
            .base=GPIOA, 
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .pullupdown=GPIO_PUPD_NONE, 
        },

        .pio_tx = {
            .base=GPIOA, 
            .pin=GPIO10,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
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
        .pio_rx = {
            .base=GPIOA, 
            .pin=GPIO2,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .pullupdown=GPIO_PUPD_NONE, 
        },

        .pio_tx = {
            .base=GPIOA, 
            .pin=GPIO3,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
    },
#endif
};
#define NUM_UARTS (sizeof(uart_configs) / sizeof(struct uart_config))


int machine_init(void)
{
    int i;
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[STM32_CLOCK]);
    gpio_create(NULL, &Led0);
    gpio_create(NULL, &Led1);
    /* Uarts */
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }
    rng_create(1, RCC_RNG);
    return 0;
}
