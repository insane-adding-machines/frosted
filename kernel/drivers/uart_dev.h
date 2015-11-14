#ifndef UART_DEV_INCLUDE /* <--- keep the same name for guard across archs for compile-time sanity */
#define UART_DEV_INCLUDE
#define UART_FR_RXFE    (0x10)
#define UART_FR_TXFF    (0x20)

#ifdef STELLARIS
#define UART_IM_RXIM    (0x10)
#else
#define UART_IM_RXIM    (0x10)
#endif
#define UART_IC_RXIC    (0x10)

#define UART_DR(baseaddr) (*(uint32_t *)(baseaddr))
#define UART_IR(baseaddr) (*(((uint32_t *)(baseaddr)) + 1))
#define UART_FR(baseaddr) (*(((uint32_t *)(baseaddr))+(0x18>>2)))
#define UART_IC(baseaddr) (*(((uint32_t *)(baseaddr))+(0x44>>2)))
#define UART_IM(baseaddr) (*(((uint32_t *)(baseaddr))+(0x38>>2)))

#define UART_RXREG UART_DR
#define UART_TXREG UART_DR

static inline char uart_rx(uint32_t *base)
{
    char byte = UART_RXREG(base);
    return byte;
}

static inline int uart_poll_rx(uint32_t *base)
{
    return (!(UART_FR(base) & UART_FR_RXFE));
}

static inline void uart_enter_irq(uint32_t *base)
{
    UART_IC(base) = UART_IC_RXIC;
}

static inline void uart_init(uint32_t *base)
{
    UART_IR(base) = 0;
    UART_IM(base) = UART_IM_RXIM;
}

static inline void uart_tx(uint32_t *base, char c)
{
    UART_TXREG(base) = c;
}
#endif
