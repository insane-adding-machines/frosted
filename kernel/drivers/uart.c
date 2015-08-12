#include "frosted.h"
#include <stdint.h>

#define UART_FR_RXFE    (0x10)
#define UART_FR_TXFF    (0x20)

#define UART_IM_RXIM    (0x10)
#define UART_IC_RXIC    (0x10)

#define UART_DR(baseaddr) (*(unsigned int *)(baseaddr))
#define UART_FR(baseaddr) (*(((unsigned int *)(baseaddr))+(0x18>>2)))
#define UART_IC(baseaddr) (*(((unsigned int *)(baseaddr))+(0x44>>2)))
#define UART_IM(baseaddr)  (*(((unsigned int *)(baseaddr))+(0x38>>2)))

void UART0_IRQHandler(void)
{
    /* Clear RX flag */
    UART_IC(UART0_BASE) = UART_IC_RXIC;
}

static int devuart_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    for (i = 0; i < len; i++) {
        UART_DR(UART0_BASE) = ch[i];
    }
    return len;
}

static int devuart_read(int fd, void *buf, unsigned int len)
{
    int out;
    char *ptr = (char *)buf;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    for(out = 0; out < len; out++) {
        /* wait for data */
        while(UART_FR(UART0_BASE) & UART_FR_RXFE);
        /* read data */
        *ptr = UART_DR(UART0_BASE);
        /* TEMP -- echo char */
        devuart_write(0, ptr, 1);
        /* CR '\n' */
        if (*(ptr) == 0xD)
            break;
        ptr++;
    }
    return out;
}


static int devuart_poll(int fd, uint16_t events)
{
    return 0;
}

static struct module mod_devuart = {
};

static struct fnode *uart = NULL;

void devuart_init(struct fnode *dev)
{
    mod_devuart.family = FAMILY_FILE;
    mod_devuart.ops.read = devuart_read; 
    mod_devuart.ops.poll = devuart_poll;
    mod_devuart.ops.write = devuart_write;
    uart = fno_create(&mod_devuart, "ttyS0", dev);
    UART_IM(UART0_BASE) = UART_IM_RXIM;
    NVIC_EnableIRQ(UART0_IRQn);

    /* Kernel printf associated to devuart_write */
    klog_set_write(devuart_write);

    klog(LOG_INFO, "UART Driver: KLOG enabled.\n");

    register_module(&mod_devuart);
}

