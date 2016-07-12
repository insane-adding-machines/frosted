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
#include <unicore-mx/cm3/systick.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/usart.h>
#include <unicore-mx/stm32/gpio.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/sdio.h>
#include "drivers/stm32f4_dsp.h"
#include "drivers/stm32_sdio.h"

#include "uart.h"
#include "gpio.h"
#include "sdio.h"
#include "dsp.h"
#include "rng.h"
#include "usb.h"
#include "eth.h"

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

static const struct gpio_config Led[4] = {
    {
        .base=GPIOD,
        .pin=GPIO12,.mode=GPIO_MODE_OUTPUT,
        .optype=GPIO_OTYPE_PP,
        .name="led0"
    },
    {
        .base=GPIOD,
        .pin=GPIO13,.mode=GPIO_MODE_OUTPUT,
        .optype=GPIO_OTYPE_PP,
        .name="led1"
    },
    {
        .base=GPIOD,
        .pin=GPIO14,.mode=GPIO_MODE_OUTPUT,
        .optype=GPIO_OTYPE_PP,
        .name="led2"
    },
    {
        .base=GPIOD,
        .pin=GPIO15,.mode=GPIO_MODE_OUTPUT,
        .optype=GPIO_OTYPE_PP,
        .name="led3"
    },
};

static const struct gpio_config Button = {
    .base=GPIOA, 
    .pin=GPIO0,
    .mode=GPIO_MODE_INPUT, 
    .optype=GPIO_OTYPE_PP, 
    .pullupdown=GPIO_PUPD_NONE, 
    .name="button"
};


static struct usb_config usb_guest = {
    .otg_mode = USB_MODE_GUEST,
    .pio_vbus = {
        .base=GPIOA,
        .pin=GPIO9,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10,
        .pullupdown=GPIO_PUPD_NONE,
    },
    .pio_dm =   {
        .base=GPIOA, 
        .pin=GPIO11,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10, 
        .pullupdown=GPIO_PUPD_NONE, 
    },
    .pio_dp =   {
        .base=GPIOA, 
        .pin=GPIO12,
        .mode=GPIO_MODE_AF,
        .af=GPIO_AF10, 
        .pullupdown=GPIO_PUPD_NONE, 
    }
};
    

const struct gpio_config stm32eth_mii_pins[] = {
    {.base=GPIOA, .pin=GPIO2, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // MDIO
    {.base=GPIOC, .pin=GPIO1, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // MDC
    {.base=GPIOA, .pin=GPIO1, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII REF CLK
    {.base=GPIOA, .pin=GPIO7, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII CRS DV
    {.base=GPIOB, .pin=GPIO10,.mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXER
    {.base=GPIOC, .pin=GPIO4, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXD0
    {.base=GPIOC, .pin=GPIO5, .mode=GPIO_MODE_AF, .af=GPIO_AF11},                        // RMII RXD1
    {.base=GPIOB, .pin=GPIO11,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXEN
    {.base=GPIOB, .pin=GPIO12,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXD0
    {.base=GPIOB, .pin=GPIO13,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11}, // RMII TXD1
};

static const struct eth_config eth_config = {
    .pio_mii = stm32eth_mii_pins,
    .n_pio_mii = 10,
    .pio_phy_reset = {
        .base=GPIOE, 
        .pin=GPIO2, 
        .mode=GPIO_MODE_OUTPUT, 
        .optype=GPIO_OTYPE_PP, 
        .pullupdown=GPIO_PUPD_PULLUP
    },
};

#if 0 /* TODO */
#ifdef CONFIG_STM32F4USB
#endif

#ifdef CONFIG_DEVSTMETH
#endif
#endif

static const struct uart_config uart_configs[] = {
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
        .pio_rx = {
            .base=GPIOD, 
            .pin=GPIO8,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .pullupdown=GPIO_PUPD_NONE, 
        },
        .pio_tx = {
            .base=GPIOD, 
            .pin=GPIO9,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF7, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
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

        .pio_rx = {
            .base=GPIOC, 
            .pin=GPIO6,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8, 
            .pullupdown=GPIO_PUPD_NONE, 
        },
        .pio_tx = {
            .base=GPIOC, 
            .pin=GPIO7,
            .mode=GPIO_MODE_AF,
            .af=GPIO_AF8, 
            .speed=GPIO_OSPEED_25MHZ, 
            .optype=GPIO_OTYPE_PP, 
        },
    },
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
    int i;
    /* Clock */
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[STM32_CLOCK]);

    /* Leds */
    for (i = 0; i < 4; i++) {
        gpio_create(NULL, &Led[i]);
    }
    
    /* Uarts */
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }
    rng_create(1, RCC_RNG);
    sdio_conf.rcc_reg = (uint32_t *)&RCC_APB2ENR;
    sdio_conf.rcc_en  = RCC_APB2ENR_SDIOEN;
    sdio_init(&sdio_conf);
    usb_init(&usb_guest);
    ethernet_init(&eth_config);

#if 0/* TODO */
    gpio_clear(GPIOE,GPIO2);    /* Clear ETH nRESET pin */
    gpio_set(GPIOE,GPIO2);      /* Set ETH nRESET pin */
#endif
}
