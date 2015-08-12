#include "frosted.h"
#include <stdint.h>

static volatile uint32_t * const TTY0WR = (uint32_t *)UART0_BASE;
static volatile uint8_t * const TTY0IE = ((uint8_t *)UART0_BASE) + 56;

static int devuart_read(int fd, void *buf, unsigned int len)
{
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    return -1; /* TODO */
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
        *TTY0WR = ch[i];
    }
    return len;
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
//    *TTY0IE = 0x04;
//    NVIC_EnableIRQ(UART0_IRQn);


    register_module(&mod_devuart);
}
