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
#include "unicore-mx/cm3/systick.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/f7/usart.h>

#if CONFIG_SYS_CLOCK == 216000000
#else
#error No valid clock speed selected for STM32F729 Discovery
#endif

#include "gpio.h"
#include "uart.h"
#include "rng.h"
#include "sdram.h"
#include "sdio.h"
#include "framebuffer.h"
#include "ltdc.h"

static const struct gpio_config gpio_led0 = {
    .base=GPIOI, 
    .pin=GPIO1,
    .mode=GPIO_MODE_OUTPUT, 
    .optype=GPIO_OTYPE_PP, 
    .name="led0"
};

static const struct uart_config uart_configs[] = {
#ifdef CONFIG_DEVUART
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
        .pio_tx = {
            .base=GPIOA, 
            .pin=GPIO10,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
        .pio_rx = {
            .base=GPIOA, 
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .pullupdown=GPIO_PUPD_NONE
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
        .pio_tx = {
            .base=GPIOA, 
            .pin=GPIO3,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
        .pio_rx = {
            .base=GPIOA, 
            .pin=GPIO2,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .pullupdown=GPIO_PUPD_NONE
        },
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
        .pio_tx = {
            .base=GPIOC, 
            .pin=GPIO6,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
        .pio_rx = {
            .base=GPIOC, 
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8, 
            .pullupdown=GPIO_PUPD_NONE
        },
    },
#endif
#endif
};
#define NUM_UARTS (sizeof(uart_configs) / sizeof(struct uart_config))


/* Setup GPIO Pins for SDIO:
   PC8 - PC11 - DAT0 thru DAT3
   PC12 - CLK
   PD2 - CMD
*/
struct sdio_config sdio_conf = {
    .pio_dat0 = {
        .base=GPIOC, 
        .pin=GPIO8,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ, 
        .af = GPIO_AF12,
        .optype=GPIO_OTYPE_PP, 
        .pullupdown=GPIO_PUPD_PULLUP

    },
    .pio_dat1 = {
        .base=GPIOC, 
        .pin=GPIO9,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ, 
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_dat2 = {
        .base=GPIOC, 
        .pin=GPIO10,
        .af = GPIO_AF12,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ, 
        .optype=GPIO_OTYPE_PP,
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_dat3 = {
        .base=GPIOC, 
        .pin=GPIO11,
        .af = GPIO_AF12,
        .mode=GPIO_MODE_AF,
        .speed=GPIO_OSPEED_100MHZ, 
        .optype=GPIO_OTYPE_PP, 
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_clk = {
        .base=GPIOC,
        .pin=GPIO12,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ, 
        .optype=GPIO_OTYPE_PP, 
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .pio_cmd = {
        .base=GPIOD,
        .pin=GPIO2,
        .mode=GPIO_MODE_AF,
        .af = GPIO_AF12,
        .speed=GPIO_OSPEED_100MHZ, 
        .optype=GPIO_OTYPE_PP, 
        .pullupdown=GPIO_PUPD_PULLUP
    },
    .card_detect_supported = 1,
    /* STM37 has an additional card-detect pin on PC13 */
    .pio_cd = {
        .base=GPIOC,
        .pin=GPIO13,
        .mode=GPIO_MODE_INPUT,
        .pullupdown=GPIO_PUPD_PULLUP
    }
};






int machine_init(void)
{
    int i = 0;
    rcc_clock_setup_hse_3v3(&hse_25mhz_3v3[CLOCK_3V3_216MHZ]);
    gpio_create(NULL, &gpio_led0);
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }
    rng_create(1, RCC_RNG);
    sdio_conf.rcc_reg = (uint32_t *)&RCC_APB2ENR;
    sdio_conf.rcc_en  = RCC_APB2ENR_SDMMC1EN;
    sdio_init(&sdio_conf);
    return 0;

}

