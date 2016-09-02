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
#include "unicore-mx/stm32/usart.h"
#include "unicore-mx/cm3/nvic.h"
#include "drivers/stm32f4_dsp.h"
#include "drivers/stm32_sdio.h"

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#include <unicore-mx/stm32/gpio.h>
#include "gpio.h"
#include "rng.h"

static const struct gpio_addr gpio_addrs[] = {
    {.base=GPIOD, .pin=GPIO12,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_12"},
    {.base=GPIOD, .pin=GPIO13,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_13"},
    {.base=GPIOD, .pin=GPIO14,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_14"},
    {.base=GPIOD, .pin=GPIO15,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_3_15"},
    {.base=GPIOA, .pin=GPIO0,.mode=GPIO_MODE_INPUT, .optype=GPIO_OTYPE_PP, .pullupdown=GPIO_PUPD_NONE, .name="gpio_0_0"},
#ifdef CONFIG_DEVUART
#ifdef CONFIG_USART_1
    {.base=GPIOA, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIOA, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_2
    {.base=GPIOA, .pin=GPIO2,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIOA, .pin=GPIO3,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_3
    {.base=GPIOD, .pin=GPIO8,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIOD, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF7, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#ifdef CONFIG_USART_6
    {.base=GPIOC, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIOC, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif
#endif
#ifdef CONFIG_STM32F4USB
    {.base=GPIOA, .pin=GPIO9,.mode=GPIO_MODE_AF,.af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
    {.base=GPIOA, .pin=GPIO11,.mode=GPIO_MODE_AF,.af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
    {.base=GPIOA, .pin=GPIO12,.mode=GPIO_MODE_AF,.af=GPIO_AF10, .pullupdown=GPIO_PUPD_NONE, .name=NULL},
#endif
#ifdef CONFIG_DEVSTMETH
    {.base=GPIOA, .pin=GPIO2, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11, .name=NULL}, // MDIO
    {.base=GPIOC, .pin=GPIO1, .mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11, .name=NULL}, // MDC
    {.base=GPIOA, .pin=GPIO1, .mode=GPIO_MODE_AF, .af=GPIO_AF11, .name=NULL},                        // RMII REF CLK
    {.base=GPIOA, .pin=GPIO7, .mode=GPIO_MODE_AF, .af=GPIO_AF11, .name=NULL},                        // RMII CRS DV
    {.base=GPIOB, .pin=GPIO10,.mode=GPIO_MODE_AF, .af=GPIO_AF11, .name=NULL},                        // RMII RXER
    {.base=GPIOC, .pin=GPIO4, .mode=GPIO_MODE_AF, .af=GPIO_AF11, .name=NULL},                        // RMII RXD0
    {.base=GPIOC, .pin=GPIO5, .mode=GPIO_MODE_AF, .af=GPIO_AF11, .name=NULL},                        // RMII RXD1
    {.base=GPIOB, .pin=GPIO11,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11, .name=NULL}, // RMII TXEN
    {.base=GPIOB, .pin=GPIO12,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11, .name=NULL}, // RMII TXD0
    {.base=GPIOB, .pin=GPIO13,.mode=GPIO_MODE_AF, .optype=GPIO_OTYPE_PP, .af=GPIO_AF11, .name=NULL}, // RMII TXD1
    {.base=GPIOE, .pin=GPIO2, .mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name=NULL, .pullupdown=GPIO_PUPD_PULLUP},            // PHY RESET
#endif
};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))

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

#ifdef CONFIG_RNG
#include "stm32_rng.h"
static const struct rng_addr rng_addrs[] = {
            {
                .devidx = 1,
                .base = 1,
                .rcc = RCC_RNG,
            },
};
#define NUM_RNGS (sizeof(rng_addrs) / sizeof(struct rng_addr))
#endif


#ifdef CONFIG_DEVSTM32SDIO
/*
 * The Embest board ties PB15 to 'card detect' which is useful
 * for aborting early, and detecting card swap. Needs porting
 * for other implementations.
 */
#define SDIO_HAS_CARD_DETECT

/*
 * Set up the GPIO pins and peripheral clocks for the SDIO
 * system. The code should probably take an option card detect
 * pin, at the moment it uses the one used by the Embest board.
 */
static void stm32_sdio_rcc_init(void)
{
    /* Enable clocks for SDIO and DMA2 */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SDIOEN);

#ifdef WITH_DMA2
    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_DMA2EN);
#endif


    /* Setup GPIO Pins for SDIO:
        PC8 - PC11 - DAT0 thru DAT3
              PC12 - CLK
               PD2 - CMD
    */
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO12 );                          // All SDIO lines are push-pull, 25Mhz
	gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO8 | GPIO9 | GPIO10 | GPIO11 ); // All SDIO lines are push-pull, 25Mhz
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO8 | GPIO9 | GPIO10 | GPIO11);            // D0 - D3 enable pullups (bi-directional)
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE,  GPIO12);                                      // CLK line no pullup
	gpio_set_af(GPIOC, GPIO_AF12, GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12);

	gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO2);
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12 | GPIO15);
	gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO2);
    gpio_set_af(GPIOD, GPIO_AF12, GPIO2);

#ifdef SDIO_HAS_CARD_DETECT
    /* SDIO Card Detect pin on the Embest Baseboard */
    /*     PB15 as a hacked Card Detect (active LOW for card present) */
    gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO15);
#endif
}

#endif /* CONFIG_DEVSTM32SDIO */


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
#       elif CONFIG_SYS_CLOCK == 180000000
        rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_180MHZ]);
#       else
#error No valid clock speed selected
#endif

    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#ifdef CONFIG_DEVUART
    uart_init(dev, uart_addrs, NUM_UARTS);
#endif
    rng_init(dev, rng_addrs, NUM_RNGS);
#ifdef CONFIG_DEVSTM32SDIO
    stm32_sdio_rcc_init();
    stm32_sdio_init(dev);
#endif
#ifdef CONFIG_DSP
    dsp_init(dev);
#endif

#ifdef CONFIG_DEVSTMETH
    gpio_clear(GPIOE,GPIO2);    /* Clear ETH nRESET pin */
    gpio_set(GPIOE,GPIO2);      /* Set ETH nRESET pin */
#endif
}
