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
#ifdef CONFIG_USART_0
         {   
             .base = USART0, 
             .irq = NVIC_UART0_IRQ, 
         },
#endif
#ifdef CONFIG_USART_1
         { 
             .base = USART1, 
             .irq = NVIC_UART1_IRQ, 
         },
#endif
#ifdef CONFIG_USART_2
         { 
             .base = USART2, 
             .irq = NVIC_UART2_IRQ, 
         },
#endif         
 };
 
#define NUM_UARTS (sizeof(uart_addrs) / sizeof(struct uart_addr))
 #endif

 void machine_init(struct fnode * dev)
 {
#ifdef CONFIG_DEVUART
     uart_init(dev, uart_addrs, NUM_UARTS);
#endif

 }
