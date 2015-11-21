#include "frosted.h"
#include <stdint.h>
#include "uart_dev.h"
//#define CONFIG_UART1
//#define CONFIG_UART3
//
//
//
//

#include "libopencm3/cm3/nvic.h"

#ifdef LM3S
#   include "libopencm3/lm3s/usart.h"
#endif


#ifndef UART0
#   define UART0 USART0
#   define UART1 USART1
#   define UART2 USART2
#endif
#ifndef NVIC_UART0_IRQ
#   define NVIC_UART0_IRQ NVIC_USART0_IRQ
#   define NVIC_UART1_IRQ NVIC_USART1_IRQ
#   define NVIC_UART2_IRQ NVIC_USART2_IRQ
#endif


#define UART_N(x) (((uint32_t)x == UART0)?0:(((uint32_t)x == UART1)?1:(((uint32_t)x == UART2)?2:3)))

static struct module mod_devuart = {
};


static int devuart_write(int fd, const void *buf, unsigned int len);

static int uart_pid[4] = {0};
static mutex_t *uart_mutex;

static uint32_t uart_check_fd(int fd)
{
    struct fnode *fno;
    fno = task_filedesc_get(fd);
    
    if (!fno)
        return 0;
    if (fd < 0)
        return 0;

    if (fno->owner != &mod_devuart)
        return 0;
    return (uint32_t)fno->priv;
}
static struct cirbuf * inbuf = NULL;

void uart_isr(uint32_t uart)
{
    /* Clear RX flag */
    usart_clear_rx_interrupt(uart);

    /* While data available */
    while (usart_rx_data_ready(uart))
    {
        char byte = (char)(usart_recv(uart) & 0xFF); 
        /* read data into circular buffer */
        cirbuf_writebyte(inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart_pid[0] > 0) 
        task_resume(uart_pid[0]);
}

void uart0_isr(void)
{
    uart_isr(UART0);
}

void usart0_isr(void)
{
    uart_isr(USART0);
}

static int devuart_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;

    uint32_t uart = uart_check_fd(fd);
    if (!uart)
        return -1;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    for (i = 0; i < len; i++) {
        usart_send(uart,ch[i]);
    }
    return len;
}


static int devuart_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct hal_iodev *dev;
    uint32_t uart;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    uart = uart_check_fd(fd);
    if (!uart)
        return -1;

    frosted_mutex_lock(uart_mutex);
    usart_disable_rx_interrupt(uart);
    len_available =  cirbuf_bytesinuse(inbuf);
    if (len_available <= 0) {
        uart_pid[UART_N(uart)] = scheduler_get_cur_pid();
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
    uart_pid[UART_N(dev)] = 0;

again:
    usart_enable_rx_interrupt(uart);
    frosted_mutex_unlock(uart_mutex);
    return out;
}


static int devuart_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct hal_iodev *dev;
    uint32_t uart = uart_check_fd(fd);
    if (!uart)
        return -1;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
    if ((events == POLLIN) && usart_rx_data_ready(uart)) {
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
    uart_mutex = frosted_mutex_init();
    mod_devuart.family = FAMILY_FILE;
    mod_devuart.ops.open = devuart_open;
    mod_devuart.ops.read = devuart_read; 
    mod_devuart.ops.poll = devuart_poll;
    mod_devuart.ops.write = devuart_write;


    /* create circular buffer */
    inbuf = cirbuf_create(256);

    uart0 = fno_create(&mod_devuart, "ttyS0", dev);
    uart0->priv = (void *)UART0;
    usart_enable_rx_interrupt(UART0);
    nvic_enable_irq(NVIC_UART0_IRQ);
    
    /* Kernel printf associated to devuart_write */
    klog_set_write(devuart_write);
    nvic_enable_irq(NVIC_UART0_IRQ);

    klog(LOG_INFO, "UART Driver: KLOG enabled.\n");

    register_module(&mod_devuart);
}

