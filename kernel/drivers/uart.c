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

static int devuart_write(int fd, const void *buf, unsigned int len);

/* Use static state for now. Future drivers can have multiple structs for this. */
static int uart_pid = 0;

struct cirbuf {
    uint8_t *buf;
    uint8_t *readptr;
    uint8_t *writeptr;
    int     bufsize;
    int     overflow;
};

static struct cirbuf * inbuf = NULL;

struct cirbuf * cirbuf_create(int size)
{
    struct cirbuf* inbuf;
    if (size <= 0) 
        return NULL;

    inbuf = f_malloc(sizeof(struct cirbuf));
    if (!inbuf)
        return NULL;

    inbuf->buf = f_malloc(size);
    if (!inbuf->buf)
    {
        f_free(inbuf);
        return NULL;
    }

    inbuf->bufsize = size;
    inbuf->readptr = inbuf->buf;
    inbuf->writeptr = inbuf->buf;
    return inbuf;
}

/* 0 on success, -1 on fail */
int cirbuf_writebyte(struct cirbuf *cb, uint8_t byte)
{
    if (!cb)
        return -1;

    /* check if there is space */
    if (!cirbuf_bytesfree(cb))
        return -1;

    *cb->writeptr++ = byte;

    /* wrap if needed */
    if (cb->writeptr > cb->buf+cb->bufsize)
        cb->writeptr = cb->buf;

    return 0;
}

/* 0 on success, -1 on fail */
int cirbuf_readbyte(struct cirbuf *cb, uint8_t *byte)
{
    if (!cb || !byte)
        return -1;

    /* check if there is data */
    if (!cirbuf_bytesinuse(cb))
        return -1;

    *byte = *cb->readptr++;

    /* wrap if needed */
    if (cb->readptr > cb->buf+cb->bufsize)
        cb->readptr = cb->buf;

    return 0;
}

/* 0 on success, -1 on fail */
int cirbuf_writebytes(struct cirbuf *cb, uint8_t * bytes, int len)
{
    uint8_t byte;
    if (!cb)
        return -1;

    /* check if there is space */
    if (!cirbuf_bytesfree(cb))
        return -1;

    /* TODO */
    // Write in 1 or 2  chunks, depending on wrap needed or not

    return len;
}

int cirbuf_bytesfree(struct cirbuf *cb)
{
    int bytes;
    if (!cb)
        return -1;

    bytes = (int)(cb->readptr - cb->writeptr - 1);
    if (cb->writeptr >= cb->readptr)
        bytes += cb->bufsize;

    return bytes;
}

int cirbuf_bytesinuse(struct cirbuf *cb)
{
    int bytes;
    if (!cb)
        return -1;

    bytes = (int)(cb->writeptr - cb->readptr);
    if (cb->writeptr < cb->readptr)
        bytes += cb->bufsize;

    return (bytes);
}

void UART0_IRQHandler(void)
{
    /* Clear RX flag */
    UART_IC(UART0_BASE) = UART_IC_RXIC;

    /* While data available */
    while (!(UART_FR(UART0_BASE) & UART_FR_RXFE))
    {
        char byte = UART_DR(UART0_BASE);
        /* read data into circular buffer */
        cirbuf_writebyte(inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart_pid > 0) 
        task_resume(uart_pid);
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
    int out, len_available;
    char *ptr = (char *)buf;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    len_available =  cirbuf_bytesinuse(inbuf);
    /*
    while (len_available <= 0) {
        task_suspend();
        len_available =  cirbuf_bytesinuse(inbuf);
    }
    */

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        cirbuf_readbyte(inbuf, ptr);
        /* TEMP -- echo char */
        if ((*ptr >= 0x20) && (*ptr <= 0x7E))
            devuart_write(0, ptr, 1);
        ptr++;
    }
    return out;
}


static int devuart_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
    if ((events == POLLIN) && (!(UART_FR(UART0_BASE) & UART_FR_RXFE))) {
        *revents |= POLLIN;
        ret = 1;
    }
    return ret;
}

static int devuart_open(const char *path, int flags)
{
    if (uart_pid != 0)
        return -1;
    uart_pid = scheduler_get_cur_pid();
}

static struct module mod_devuart = {
};

static struct fnode *uart = NULL;

void devuart_init(struct fnode *dev)
{
    mod_devuart.family = FAMILY_FILE;
    mod_devuart.ops.open = devuart_open;
    mod_devuart.ops.read = devuart_read; 
    mod_devuart.ops.poll = devuart_poll;
    mod_devuart.ops.write = devuart_write;


    /* create circular buffer */
    inbuf = cirbuf_create(256);

    uart = fno_create(&mod_devuart, "ttyS0", dev);
    UART_IM(UART0_BASE) = UART_IM_RXIM;
    NVIC_EnableIRQ(UART0_IRQn);

    /* Kernel printf associated to devuart_write */
    klog_set_write(devuart_write);

    klog(LOG_INFO, "UART Driver: KLOG enabled.\n");

    register_module(&mod_devuart);
}

