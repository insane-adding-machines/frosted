#ifndef INC_UART
#define INC_UART

struct uart_addr {
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    //TBD
    //TX Pin
    //RX Pin
    //CTS Pin
    //RTS Pin
};

int uart_fno_init(struct fnode *dev, uint32_t n, struct uart_addr * addr);
struct module * devuart_init(struct fnode *dev);

#endif

