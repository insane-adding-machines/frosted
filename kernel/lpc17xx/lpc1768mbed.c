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
#include <libopencm3/lpc17xx/uart.h>
#include "uart.h"
#endif

#ifdef CONFIG_DEVGPIO
#include <libopencm3/lpc17xx/gpio.h>
#include "gpio.h"
#endif

#ifdef CONFIG_DEVGPIO
static const struct gpio_addr gpio_addrs[] = {                                  
    {.base=GPIO1, .pin=GPIOPIN18, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_18", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN20, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_20", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN21, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_21", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN23, .mode=GPIO_MODE_OUTPUT, .name="gpio_1_23", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */}, 
#ifdef CONFIG_DEVUART
#ifdef CONFIG_UART_0
    {.base=GPIO0, .pin=GPIOPIN3,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIO0, .pin=GPIOPIN2,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .name=NULL,},
#endif
#ifdef CONFIG_UART_1
    {.base=GPIO0, .pin=GPIOPIN16,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIO0, .pin=GPIOPIN15,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .name=NULL,},
#endif
#ifdef CONFIG_UART_2
    {.base=GPIO0, .pin=GPIOPIN11,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIO0, .pin=GPIOPIN10,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .name=NULL,},
#endif
#ifdef CONFIG_UART_3
    {.base=GPIO0, .pin=GPIO1,.mode=GPIO_MODE_AF,.af=GPIO_AF2, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
    {.base=GPIO0, .pin=GPIO0,.mode=GPIO_MODE_AF,.af=GPIO_AF2, .name=NULL,},
#endif
#endif
};
#define NUM_GPIOS (sizeof(gpio_addrs) / sizeof(struct gpio_addr))
#endif

#ifdef CONFIG_DEVUART
/* TODO: Move to libopencm3 when implemented */



void usart_set_baudrate(uint32_t usart, uint32_t baud)
{
    /* TODO */
    (void)usart;
    (void)baud;
}

void usart_set_databits(uint32_t usart, int bits)
{
    /* TODO */
    (void)usart;
    (void)bits;
}

void usart_set_stopbits(uint32_t usart, enum usart_stopbits sb)
{
    /* TODO */
    (void)usart;
    (void)sb;
}

void usart_set_parity(uint32_t usart, enum usart_parity par)
{
    /* TODO */
    (void)usart;
    (void)par;
}

void usart_set_mode(uint32_t usart, enum usart_mode mode)
{
    /* TODO */
    (void)usart;
    (void)mode;
}

void usart_set_flow_control(uint32_t usart, enum usart_flowcontrol fc)
{
    /* TODO */
    (void)usart;
    (void)fc;
}

void usart_enable(uint32_t usart)
{
       (void)usart;
}

void usart_disable(uint32_t usart)
{
       (void)usart;
}



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
#endif

void machine_init(struct fnode * dev)
{

#if CONFIG_SYS_CLOCK == 100000000
     clock_setup(&clock_scale[CLOCK_96MHZ]);
#else
#error No valid clock speed selected for lpc1768mbed
#endif

#ifdef CONFIG_DEVGPIO
    gpio_init(dev, gpio_addrs, NUM_GPIOS);
#endif
#ifdef CONFIG_DEVUART
   uart_init(dev, uart_addrs, NUM_UARTS);
#endif

}

