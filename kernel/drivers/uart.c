#include "frosted.h"
#include <stdint.h>

extern const uint32_t UART0_BASE;
extern const uint32_t UART0_IRQn;

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
static mutex_t *uart_mutex;
static struct cirbuf * inbuf = NULL;

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
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    mutex_lock(uart_mutex);
    hal_irq_off(UART0_IRQn);
    len_available =  cirbuf_bytesinuse(inbuf);
    if (len_available <= 0) {
        uart_pid = scheduler_get_cur_pid();
        task_suspend();
        out = SYS_CALL_AGAIN;
        goto again;
    }

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(inbuf, ptr) != 0)
            break;
        ptr++;
    }
    uart_pid = 0;

again:
    hal_irq_on(UART0_IRQn);
    mutex_unlock(uart_mutex);
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
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    return task_filedesc_add(f); 
}

static struct module mod_devuart = {
};

static struct fnode *uart = NULL;

void devuart_init(struct fnode *dev)
{
    uart_mutex = mutex_init();
    mod_devuart.family = FAMILY_FILE;
    mod_devuart.ops.open = devuart_open;
    mod_devuart.ops.read = devuart_read; 
    mod_devuart.ops.poll = devuart_poll;
    mod_devuart.ops.write = devuart_write;


    /* create circular buffer */
    inbuf = cirbuf_create(256);

    uart = fno_create(&mod_devuart, "ttyS0", dev);
    UART_IM(UART0_BASE) = UART_IM_RXIM;
    hal_irq_set_prio(UART0_IRQn, OS_IRQ_PRIO);
    hal_irq_on(UART0_IRQn);

    /* Kernel printf associated to devuart_write */
    klog_set_write(devuart_write);

    klog(LOG_INFO, "UART Driver: KLOG enabled.\n");

    register_module(&mod_devuart);
}

