#ifndef INC_UART
#define INC_UART

#include "gpio.h"

/* TX, RX, RTS, CTS, CK*/
#define MAX_UART_PINS 5

struct uart_addr {
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    uint8_t num_pins;
    struct gpio_addr pins[MAX_UART_PINS];
};

int uart_fno_init(struct fnode *dev, uint32_t n, const struct uart_addr * addr);
struct module * devuart_init(struct fnode *dev);

#endif

