#ifndef INC_UART
#define INC_UART

#include "frosted.h"
#include "gpio.h"

/* TX, RX, RTS, CTS, CK*/
#define MAX_UART_PINS 5

struct uart_config {
    uint8_t devidx;
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    uint32_t baudrate;
    uint8_t stop_bits;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t flow;
    struct gpio_config pio_rx;
    struct gpio_config pio_tx;
    struct gpio_config pio_cts;
    struct gpio_config pio_rts;
};

#ifdef CONFIG_DEVUART
int uart_init(void);
int uart_create(const struct uart_config *cfg);
#else
#define uart_init() (-ENOENT)
#define uart_create(x) (-ENOENT)
#endif


#endif

