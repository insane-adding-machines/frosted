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
#include "libopencm3/lm3s/usart.h"
#include "libopencm3/lm3s/nvic.h"

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif


#ifdef CONFIG_DEVUART
 static const struct uart_addr uart_addrs[] = { 
         {   
             .base = USART0, 
             .irq = NVIC_UART0_IRQ, 
         },
         { 
             .base = USART1, 
             .irq = NVIC_UART1_IRQ, 
         },
         { 
             .base = USART2, 
             .irq = NVIC_UART2_IRQ, 
         },
 };
 
#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))
 
 static void uart_init(struct fnode * dev)
 {
     int i,j;
     const struct gpio_addr * pcfg;
     struct module * devuart = devuart_init(dev);
 
     for (i = 0; i < NUM_UARTS; i++) 
     {
         uart_fno_init(dev, i, &uart_addrs[i]);
     }
     register_module(devuart);
 }
#endif

 void machine_init(struct fnode * dev)
 {
#ifdef CONFIG_DEVUART
     uart_init(dev);
#endif

 }
