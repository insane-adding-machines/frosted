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
#include <unicore-mx/lpc17xx/clock.h>
#include <unicore-mx/lpc17xx/nvic.h>
#include <unicore-mx/lpc17xx/pwr.h>
#include <unicore-mx/lpc17xx/uart.h>
#include <unicore-mx/lpc17xx/gpio.h>
#include "gpio.h"
#include "uart.h"


#if CONFIG_SYS_CLOCK == 100000000
    const uint32_t clock_96MHZ = CLOCK_96MHZ;
#else
#error No valid clock speed selected for lpc1768mbed
#endif

static const struct gpio_config Leds[] = {                                  
    {.base=GPIO1, .pin=GPIOPIN18, .mode=GPIO_MODE_OUTPUT, .name="led0", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN20, .mode=GPIO_MODE_OUTPUT, .name="led1", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN21, .mode=GPIO_MODE_OUTPUT, .name="led2", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */},
    {.base=GPIO1, .pin=GPIOPIN23, .mode=GPIO_MODE_OUTPUT, .name="led3", /* .exti=1, .trigger=EXTI_TRIGGER_RISING */}, 
};
#define NUM_LEDS (sizeof(Leds) / sizeof(struct gpio_config))

static const struct uart_config uart_configs[] = {
#ifdef CONFIG_USART_0
    {
        .devidx = 0,
        .base = UART0_BASE,
        .irq = NVIC_UART0_IRQ,
        .rcc = PWR_PCONP_UART0,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_rx = {.base=GPIO0, .pin=GPIOPIN3,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
        .pio_tx = {.base=GPIO0, .pin=GPIOPIN2,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .name=NULL,},

    },
#endif
#ifdef CONFIG_USART_1
    {
        .devidx = 1,
        .base = UART1_BASE,
        .irq = NVIC_UART1_IRQ,
        .rcc = PWR_PCONP_UART1,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_rx = {.base=GPIO2, .pin=GPIOPIN1,.mode=GPIO_MODE_AF,.af=GPIO_AF2, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
        .pio_tx = {.base=GPIO2, .pin=GPIOPIN0,.mode=GPIO_MODE_AF,.af=GPIO_AF2, .name=NULL,},

    },
#endif
#ifdef CONFIG_USART_2
    {
        .devidx = 2,
        .base = UART2_BASE,
        .irq = NVIC_UART2_IRQ,
        .rcc = PWR_PCONP_UART2,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_rx = {.base=GPIO0, .pin=GPIOPIN11,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
        .pio_tx = {.base=GPIO0, .pin=GPIOPIN10,.mode=GPIO_MODE_AF,.af=GPIO_AF1, .name=NULL,},
    },
#endif
#ifdef CONFIG_USART_3
    {
        .devidx = 3,
        .base = UART3_BASE,
        .irq = NVIC_UART3_IRQ,
        .rcc = PWR_PCONP_UART3,
        .baudrate = 115200,
        .stop_bits = USART_STOPBITS_1,
        .data_bits = 8,
        .parity = USART_PARITY_NONE,
        .flow = USART_FLOWCONTROL_NONE,
        .pio_rx = {.base=GPIO0, .pin=GPIOPIN26,.mode=GPIO_MODE_AF,.af=GPIO_AF3, .pullupdown=GPIO_PUPD_NONE, .name=NULL,},
        .pio_tx = {.base=GPIO0, .pin=GPIOPIN25,.mode=GPIO_MODE_AF,.af=GPIO_AF3, .name=NULL,},
    },
#endif
};
#define NUM_UARTS (sizeof(uart_configs) / sizeof(struct uart_config))

/* TODO: Move to unicore-mx when implemented */

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


int machine_init(void)
{
    int i;
    clock_setup(&clock_scale[clock_96MHZ]);

    /* Leds */
    for (i = 0; i < 4; i++) {
        gpio_create(NULL, &Leds[i]);
    }

    /* Uarts */
    for (i = 0; i < NUM_UARTS; i++) {
        uart_create(&uart_configs[i]);
    }
    return 0;

}

