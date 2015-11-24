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

#ifdef CONFIG_GPIO_LPC17XX
#include <libopencm3/lpc17xx/gpio.h>
#include "gpio.h"


static const struct gpio_addr gpio_addrs[] = {   {.port=GPIO1, .pin=GPIOPIN18, .name="a_pio_1_18"},
                                                                                {.port=GPIO1, .pin=GPIOPIN20, .name="a_pio_1_20"},
                                                                                {.port=GPIO1, .pin=GPIOPIN21, .name="a_pio_1_21"},
                                                                                {.port=GPIO1, .pin=GPIOPIN23, .name="a_pio_1_23"} };

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
        { .base = UART0_BASE, .irq = NVIC_UART0_IRQ, },
        { .base = UART1_BASE, .irq = NVIC_UART1_IRQ, },
        { .base = UART2_BASE, .irq = NVIC_UART2_IRQ, },
        { .base = UART3_BASE, .irq = NVIC_UART3_IRQ, },
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
        /* Enable the external clock */
        CLK_SCS |= 0x20;                
        while((CLK_SCS & 0x40) == 0);
        /* Select external oscilator */
        CLK_CLKSRCSEL = 0x01;   
        /*N = 2 & M = 25*/
        CLK_PLL0CFG = (2<<16) |25;
        /*Feed*/
        CLK_PLL0FEED=0xAA; 
        CLK_PLL0FEED=0x55;
        /*PLL0 Enable */
        CLK_PLL0CON = 1;
        /*Feed*/
        CLK_PLL0FEED=0xAA; 
        CLK_PLL0FEED=0x55;
        /* Divide by 3 */
        CLK_CCLKCFG = 2;
        /* wait until locked */
        while (!(CLK_PLL0STAT & (1<<26)));
        /* see flash accelerator - TBD*/
        /*_FLASHCFG = (_FLASHCFG & 0xFFF) | (4<<12);*/
        /* PLL0 connect */
        CLK_PLL0CON |= 1<<1;
        /*Feed*/
        CLK_PLL0FEED=0xAA; 
        CLK_PLL0FEED=0x55;
        /* PLL0 operational */
#else
#error No valid clock speed selected for lpc1768mbed
#endif

#ifdef CONFIG_GPIO_LPC17XX
    gpio_init(dev);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev);
#endif

}

