#ifndef INC_UART
#define INC_UART

#include "gpio.h"

/* TX, RX, RTS, CTS, CK*/
#define MAX_UART_PINS 5

struct uart_addr {
    uint8_t devidx;
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    uint32_t baudrate;
    uint8_t stop_bits;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t flow;
};

int uart_fno_init(struct fnode *dev, uint32_t n, const struct uart_addr * addr);
struct module * devuart_init(struct fnode *dev);

#endif

