#include "frosted.h"
#include <stdint.h>
#include "uart_dev.h"
//#define CONFIG_UART1
//#define CONFIG_UART3

extern struct hal_iodev UART0;
extern struct hal_iodev UART1;
extern struct hal_iodev UART2;
extern struct hal_iodev UART3;

static int uart_n(struct hal_iodev *uart)
{
    if (0) {}
#ifdef CONFIG_UART1 
    else if (uart == &UART1)
        return 1;
#endif

#ifdef CONFIG_UART2 
    else if (uart == &UART2)
        return 2;
#endif

#ifdef CONFIG_UART3 
    if (uart == &UART3)
        return 3;
#endif
    return 0;
}

static struct module mod_devuart = {
};


static int devuart_write(int fd, const void *buf, unsigned int len);

static int uart_pid[4] = {0};
static mutex_t *uart_mutex;

static int uart_check_fd(int fd, struct hal_iodev **iodev)
{
    struct fnode *fno;
    fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;
    if (fd < 0)
        return -1;

    if (fno->owner != &mod_devuart)
        return -1;
    *iodev = (struct hal_iodev *)fno->priv;
    return 0;
}
static struct cirbuf * inbuf = NULL;

void UART0_Handler(void)
{
    /* Clear RX flag */
    uart_enter_irq(UART0.base);

    /* While data available */
    while (uart_poll_rx(UART0.base))
    {
        char byte = uart_rx(UART0.base); 
        /* read data into circular buffer */
        cirbuf_writebyte(inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart_pid[0] > 0) 
        task_resume(uart_pid[0]);
}
#ifdef CONFIG_UART1
void UART1_Handler(void)
{
    /* Clear RX flag */
    uart_enter_irq(UART1.base);

    /* While data available */
    while (uart_poll_rx(UART1.base))
    {
        char byte = uart_rx(UART1.base); 
        /* read data into circular buffer */
        cirbuf_writebyte(inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart_pid[1] > 0) 
        task_resume(uart_pid[1]);
}
#endif

#ifdef CONFIG_UART3
void UART3_Handler(void)
{
    /* Clear RX flag */
    uart_enter_irq(UART3.base);

    /* While data available */
    while (uart_poll_rx(UART3.base))
    {
        char byte = uart_rx(UART3.base); 
        /* read data into circular buffer */
        cirbuf_writebyte(inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart_pid[3] > 0) 
        task_resume(uart_pid[3]);
}
#endif

static int devuart_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    struct hal_iodev *dev;

    if (uart_check_fd(fd, &dev) != 0)
        return -1;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    for (i = 0; i < len; i++) {
        uart_tx((dev->base),ch[i]);
    }
    return len;
}


static int devuart_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct hal_iodev *dev;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    if (uart_check_fd(fd, &dev) != 0)
        return -1;

    mutex_lock(uart_mutex);
    hal_irq_off(dev->irqn);
    len_available =  cirbuf_bytesinuse(inbuf);
    if (len_available <= 0) {
        uart_pid[uart_n(dev)] = scheduler_get_cur_pid();
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
    uart_pid[uart_n(dev)] = 0;

again:
    hal_irq_on(dev->irqn);
    mutex_unlock(uart_mutex);
    return out;
}


static int devuart_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct hal_iodev *dev;
    if (uart_check_fd(fd, &dev) != 0)
        return -1;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
    if ((events == POLLIN) && (!(UART_FR(dev->base) & UART_FR_RXFE))) {
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

static struct fnode *uart0 = NULL;
static struct fnode *uart3 = NULL;

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

    uart0 = fno_create(&mod_devuart, "ttyS0", dev);
    uart0->priv = &UART0;
    uart_init(UART0.base);
    hal_iodev_on(&UART0);
  
#ifdef CONFIG_UART1 
    uart3 = fno_create(&mod_devuart, "ttyS1", dev);
    uart3->priv = &UART1;
    uart_init(UART1.base);
    hal_iodev_on(&UART0);
#endif
#ifdef CONFIG_UART3 
    lpc1768_pio_mode(0, 25, 2<<2); 
    lpc1768_pio_func(0, 25, 3);
    lpc1768_pio_mode(0, 26, 2<<2); 
    lpc1768_pio_func(0, 26, 3);
    
    uart3 = fno_create(&mod_devuart, "ttyS3", dev);
    uart3->priv = &UART3;
    uart_init(UART3.base);
    hal_iodev_on(&UART0);
#endif

    /* Kernel printf associated to devuart_write */
    klog_set_write(devuart_write);

    klog(LOG_INFO, "UART Driver: KLOG enabled.\n");

    register_module(&mod_devuart);
}

