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

static const struct gpio_addr a_pio_3_12 = {GPIOD, GPIO12};
static const struct gpio_addr a_pio_3_13 = {GPIOD, GPIO13};
static const struct gpio_addr a_pio_3_14 = {GPIOD, GPIO14};
static const struct gpio_addr a_pio_3_15 = {GPIOD, GPIO15};

static void gpio_init(struct fnode * dev)
{
    struct fnode *gpio_3_12;
    struct fnode *gpio_3_13;
    struct fnode *gpio_3_14;
    struct fnode *gpio_3_15;

    rcc_periph_clock_enable(RCC_GPIOD);

    struct module * devgpio = devgpio_init(dev);

    gpio_3_12 = fno_create(devgpio, "gpio_3_12", dev);
    if (gpio_3_12)
        gpio_3_12->priv = &a_pio_3_12;

    gpio_3_13 = fno_create(devgpio, "gpio_3_13", dev);
    if (gpio_3_13)
        gpio_3_13->priv = &a_pio_3_13;

    gpio_3_14 = fno_create(devgpio, "gpio_3_14", dev);
    if (gpio_3_14)
        gpio_3_14->priv = &a_pio_3_14;

    gpio_3_15 = fno_create(devgpio, "gpio_3_15", dev);
    if (gpio_3_15)
        gpio_3_15->priv = &a_pio_3_15;

    register_module(devgpio);

}
#endif

#ifdef CONFIG_DEVUART

//401           - 3 USART
//410/411   - 3 USART 1,2,6
//415/417   - 4 USART 1,2,3,6 
//                    2 UART 4,5 
//405           - 6 USART
//407           - 6 USART
//446           - 4 USART 
//                    2 UART 
//427/437   - 4 USART
//                    4 UART
//429/439   - 4 USART
//                    4 UART
//469/479   - 4 USART
//                    4 UART
                        
static const struct uart_addr uart_addrs[] = { 
        { .base = USART1, .irq = NVIC_USART1_IRQ, .rcc = RCC_USART1, },
        { .base = USART2, .irq = NVIC_USART2_IRQ, .rcc = RCC_USART2, },
        { .base = USART6, .irq = NVIC_USART6_IRQ, .rcc = RCC_USART6, },
};

#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))

static void uart_init(struct fnode * dev)
{
    int i;
    struct module * devuart = devuart_init(dev);

    for (i = 0; i < NUM_UARTS; i++) 
    {
        uart_fno_init(dev, i, &uart_addrs[i]);
        usart_enable(uart_addrs[i].rcc);
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

