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
 *      Authors: Roel Postelmans
 *
 */
#include "frosted.h"
#include "unicore-mx/nrf/52/uart.h"
#include <unicore-mx/nrf/52/gpio.h>
#include "gpio.h"
#include "uart.h"


static const struct gpio_config Led0 = {
    .base=GPIO,
    .pin=GPIO19,
    .mode=GPIO_DIR_OUTPUT,
    .name="led0"
};

static const struct uart_config uart_configs[] = {
#ifdef CONFIG_DEVUART
#ifdef CONFIG_UART_0
    {
        .devidx = 0,
        .base = UART0,
        .irq = NVIC_UART0_IRQ,
        .baudrate = UART_BAUD_115200,
        .stop_bits = 1,
        .data_bits = 8,
        .parity = 0,
        .flow = 0,
        .pio_tx = {
            .base=GPIO,
            .pin=GPIO9,
            .mode=GPIO_MODE_INPUT,
        },
        .pio_rx = {
            .base=GPIO,
            .pin=GPIO11,
            .mode=GPIO_MODE_OUTPUT,
            .pullupdown=GPIO_PUPD_NONE,
        },
    },
#endif
#endif
};

int machine_init(void)
{
    gpio_create(NULL, &Led0);
    uart_create(&uart_configs[0]);

    return 0;
}
