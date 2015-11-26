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

#include "libopencm3/cm3/systick.h"
#include <libopencm3/stm32/rcc.h>
#include "libopencm3/stm32/usart.h"
#include "libopencm3/cm3/nvic.h"

#include "stm32f4.h"

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_DEVUART
void uart_init(struct fnode * dev, const struct uart_addr uart_addrs[], int num_uarts)
{
    int i,j;
    struct module * devuart = devuart_init(dev);

    for (i = 0; i < num_uarts; i++) 
    {
        rcc_periph_clock_enable(uart_addrs[i].rcc);
        uart_fno_init(dev, uart_addrs[i].devidx, &uart_addrs[i]);
        usart_set_baudrate(uart_addrs[i].base, uart_addrs[i].baudrate);
        usart_set_databits(uart_addrs[i].base, uart_addrs[i].data_bits);
        usart_set_stopbits(uart_addrs[i].base, uart_addrs[i].stop_bits);
        usart_set_mode(uart_addrs[i].base, USART_MODE_TX_RX);
        usart_set_parity(uart_addrs[i].base, uart_addrs[i].parity);
        usart_set_flow_control(uart_addrs[i].base, uart_addrs[i].flow);
        /* one day we will do non blocking UART Tx and will need to enable tx interrupt */
        usart_enable_rx_interrupt(uart_addrs[i].base);
        /* Finally enable the USART. */
        usart_enable(uart_addrs[i].base);
    }
    register_module(devuart);
}
#endif





