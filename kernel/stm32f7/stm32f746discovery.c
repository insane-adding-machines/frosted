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
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/f7/usart.h>

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_DEVGPIO
#include <libopencm3/stm32/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVFRAMEBUFFER
#include "framebuffer.h"
#endif

#ifdef CONFIG_DEVGPIO
static const struct gpio_addr gpio_addrs[] = { {.base=GPIOI, .pin=GPIO1,.mode=GPIO_MODE_OUTPUT, .optype=GPIO_OTYPE_PP, .name="gpio_9_1"},

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
#endif
#ifdef CONFIG_UART_4
#endif
#ifdef CONFIG_UART_5
#endif
#ifdef CONFIG_USART_6
    {.base=GPIOC, .pin=GPIO6,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .speed=GPIO_OSPEED_25MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
    {.base=GPIOC, .pin=GPIO7,.mode=GPIO_MODE_AF,.af=GPIO_AF8, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
#endif
#ifdef CONFIG_UART_7
#endif
#ifdef CONFIG_UART_8
#endif
#endif

#define CONFIG_SDRAM
#ifdef CONFIG_SDRAM
//    {.base=GPIOC, .pin=GPIO3, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO0, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO1, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO3, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO8, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO9, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO14,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOD, .pin=GPIO15,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO0, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO1, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO7, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO8, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO9, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO10,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO11,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO12,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO13,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO14,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOE, .pin=GPIO15,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO0, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO1, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO2, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO3, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO4, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO5, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO11,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO12,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO13,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO14,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOF, .pin=GPIO15,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO0, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO1, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO4, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO5, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO8, .mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOG, .pin=GPIO15,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOH, .pin=GPIO3 ,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
//    {.base=GPIOH, .pin=GPIO5 ,.mode=GPIO_MODE_AF,.af=GPIO_AF12,.pullupdown=GPIO_PUPD_PULLUP, .speed=GPIO_OSPEED_100MHZ, .optype=GPIO_OTYPE_PP, .name=NULL,},
#endif

};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif

#ifdef CONFIG_DEVUART
static const struct uart_addr uart_addrs[] = { 
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
#include "random.h"
static const struct rng_addr rng_addrs[] = {
            {
                .devidx = 1,
                .base = 1,
                .rcc = RCC_RNG,
            },
};
#define NUM_RNGS (sizeof(rng_addrs) / sizeof(struct rng_addr))
#endif


#ifdef CONFIG_DEVFRAMEBUFFER
void lcd_pinmux(void)
{
    /* Enable the LTDC Clock */
    rcc_periph_clock_enable(RCC_LTDC);

    /* Enable GPIOs clock */
    rcc_periph_clock_enable(RCC_GPIOE);
    rcc_periph_clock_enable(RCC_GPIOG);
    rcc_periph_clock_enable(RCC_GPIOI);
    rcc_periph_clock_enable(RCC_GPIOJ);
    rcc_periph_clock_enable(RCC_GPIOK);

    /*** LTDC Pins configuration ***/
    gpio_mode_setup(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
    gpio_set_output_options(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO4);
    gpio_set_af(GPIOE, GPIO_AF14, GPIO4);

    /* GPIOG configuration */
    gpio_mode_setup(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12);
    gpio_set_output_options(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO12);
    gpio_set_af(GPIOG, GPIO_AF9, GPIO12);

    /* GPIOI LTDC alternate configuration */
    gpio_mode_setup(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15);
    gpio_set_output_options(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15);
    gpio_set_af(GPIOI, GPIO_AF14, GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15);

    /* GPIOJ configuration */
    gpio_mode_setup(GPIOJ, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 |
                                                         GPIO5 | GPIO6 | GPIO7 | GPIO8 | GPIO9 |
                                                         GPIO10 | GPIO11 | GPIO13 | GPIO14 | GPIO15);
    gpio_set_output_options(GPIOJ, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 |
                                                         GPIO5 | GPIO6 | GPIO7 | GPIO8 | GPIO9 |
                                                         GPIO10 | GPIO11 | GPIO13 | GPIO14 | GPIO15);
    gpio_set_af(GPIOJ, GPIO_AF14, GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 |
                                                         GPIO5 | GPIO6 | GPIO7 | GPIO8 | GPIO9 |
                                                         GPIO10 | GPIO11 | GPIO13 | GPIO14 | GPIO15);

    /* GPIOK configuration */
    gpio_mode_setup(GPIOK, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_set_output_options(GPIOK, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO0 | GPIO1 | GPIO2 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
    gpio_set_af(GPIOK, GPIO_AF14, GPIO0 | GPIO1 | GPIO2 | GPIO4 | GPIO5 | GPIO6 | GPIO7);
  
    /* LCD_DISP GPIO configuration */
    gpio_mode_setup(GPIOI, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);
    gpio_set_output_options(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO12);

    /* LCD_BL_CTRL GPIO configuration */
    gpio_mode_setup(GPIOK, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO3);
    gpio_set_output_options(GPIOK, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, GPIO3);
    /* Assert display enable LCD_DISP pin */
    gpio_set(GPIOI, GPIO12);
    /* Assert backlight LCD_BL_CTRL pin */
    gpio_set(GPIOK, GPIO3);
}
#endif



void machine_init(struct fnode * dev)
{
#       if CONFIG_SYS_CLOCK == 216000000
            rcc_clock_setup_hse_3v3(&hse_25mhz_3v3[CLOCK_3V3_216MHZ]);
#       else
#error No valid clock speed selected for STM32F729 Discovery
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev, uart_addrs, NUM_UARTS);
#endif
#ifdef CONFIG_RNG
    rng_init(dev, rng_addrs, NUM_RNGS);
#endif
#ifdef CONFIG_SDRAM
    extern void sdram_init(void);
    sdram_init();
#endif

#ifdef CONFIG_DEVFRAMEBUFFER
    lcd_pinmux();
    stm32f7_ltdc_init();
#endif

}

