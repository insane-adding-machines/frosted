#ifndef INC_LPC17CC
#define INC_LPC17CC

#ifdef CONFIG_DEVUART
#include "uart.h"
#endif

#ifdef CONFIG_DEVUART
void uart_init(struct fnode * dev, const struct uart_addr uart_addrs[], int num_uarts);
#endif

#endif
