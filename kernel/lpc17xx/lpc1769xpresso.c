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
#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_GPIO_LPC17XX
#include <libopencm3/lpc17xx/gpio.h>
#include "gpio.h"

static const struct gpio_addr gpio_addrs[] = {   {.port=GPIO0, .pin=GPIOPIN22, .name="a_pio_0_22"} };

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
#ifdef CONFIG_UART_0
        { .base = UART0_BASE, .irq = NVIC_UART0_IRQ, },
#endif
#ifdef CONFIG_UART_1
        { .base = UART1_BASE, .irq = NVIC_UART1_IRQ, },
#endif
#ifdef CONFIG_UART_2
        { .base = UART2_BASE, .irq = NVIC_UART2_IRQ, },
#endif
#ifdef CONFIG_UART_3
        { .base = UART3_BASE, .irq = NVIC_UART3_IRQ, },
#endif
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
/*N = 3 & M = 25*/
#define PLL0CFG_VALUE ((2<<16) |25)
#elif CONFIG_SYS_CLOCK == 120000000
/*N = 3 & M = 30*/
#define PLL0CFG_VALUE ((2<<16) |30)
#else
#error No valid clock speed selected for lpc1768mbed
#endif

        /* Enable the external clock */
        CLK_SCS |= 0x20;                
        while((CLK_SCS & 0x40) == 0);
        /* Select external oscilator */
        CLK_CLKSRCSEL = 0x01;   

        CLK_PLL0CFG = PLL0CFG_VALUE;
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

#ifdef CONFIG_GPIO_LPC17XX
    gpio_init(dev);
#endif
#ifdef CONFIG_DEVUART
    uart_init(dev);
#endif

}

