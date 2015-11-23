#include "frosted.h"
#include <stdint.h>
#include "uart_dev.h"

#include "libopencm3/cm3/nvic.h"

#ifdef LM3S
#   include "libopencm3/lm3s/usart.h"
#endif
#ifdef STM32F4
#   include "libopencm3/stm32/f4/memorymap.h"
#   include "libopencm3/stm32/f4/usart.h"
#   define usart_clear_rx_interrupt(x) do{}while(0)
#endif


#ifndef UART0
#ifdef USART0
#   define UART0 USART0
#else 
#   define UART0 (0)
#endif
#endif

#ifndef UART1
#ifdef USART1
#   define UART1 USART1
#else 
#   define UART1 (0)
#endif
#endif

#ifndef UART2
#ifdef USART2
#   define UART2 USART2
#else 
#   define UART2 (0)
#endif
#endif
#ifndef UART3
#ifdef USART3
#   define UART3 USART3
#else 
#   define UART3 (0)
#endif
#endif

#ifdef NVIC_USART0_IRQ
#   define NVIC_UART0_IRQ NVIC_USART0_IRQ
#endif
#ifdef NVIC_USART1_IRQ
#   define NVIC_UART1_IRQ NVIC_USART1_IRQ
#endif
#ifdef NVIC_USART2_IRQ
#   define NVIC_UART2_IRQ NVIC_USART2_IRQ
#endif
#ifdef NVIC_USART3_IRQ
#   define NVIC_UART3_IRQ NVIC_USART2_IRQ
#endif

#ifndef NVIC_UART0_IRQ
#define NVIC_UART0_IRQ 0
#endif
#ifndef NVIC_UART1_IRQ
#define NVIC_UART1_IRQ 0
#endif
#ifndef NVIC_UART2_IRQ
#define NVIC_UART2_IRQ 0
#endif
#ifndef NVIC_UART3_IRQ
#define NVIC_UART3_IRQ 0
#endif


static struct module mod_devuart = {
};

struct dev_uart {
    uint32_t base;
    uint32_t irq;
    struct fnode *fno;
    struct cirbuf *inbuf;
    mutex_t *mutex;
    uint16_t pid;
};

static struct dev_uart DEV_UART[4] = { 
        { .base = UART0, .irq = NVIC_UART0_IRQ, },
        { .base = UART1, .irq = NVIC_UART1_IRQ, },
        { .base = UART2, .irq = NVIC_UART2_IRQ, },
        { .base = UART3, .irq = NVIC_UART3_IRQ, },
};

static int devuart_write(int fd, const void *buf, unsigned int len);


static struct dev_uart *uart_check_fd(int fd)
{
    struct fnode *fno;
    fno = task_filedesc_get(fd);
    
    if (!fno)
        return 0;
    if (fd < 0)
        return 0;

    if (fno->owner != &mod_devuart)
        return 0;
    return fno->priv;
}

void uart_isr(struct dev_uart *uart)
{
    /* Clear RX flag */
    usart_clear_rx_interrupt(uart->base);

    /* While data available */
    while (usart_is_recv_ready(uart->base))
    {
        char byte = (char)(usart_recv(uart->base) & 0xFF); 
        /* read data into circular buffer */
        cirbuf_writebyte(uart->inbuf, byte);
    }

    /* If a process is attached, resume the process */
    if (uart->pid > 0) 
        task_resume(uart->pid);
}

void uart0_isr(void)
{
    uart_isr(&DEV_UART[0]);
}

void uart1_isr(void)
{
    uart_isr(&DEV_UART[1]);
}

#ifdef USART0
void usart0_isr(void)
{
    uart_isr(&DEV_UART[0]);
}
#endif

#ifdef USART1
void usart1_isr(void)
{
    uart_isr(&DEV_UART[1]);
}
#endif

static int devuart_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    struct dev_uart *uart;

    uart = uart_check_fd(fd);
    if (!uart)
        return -1;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    for (i = 0; i < len; i++) {
        usart_send(uart->base,ch[i]);
    }
    return len;
}


static int devuart_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct hal_iodev *dev;
    struct dev_uart *uart;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    uart = uart_check_fd(fd);
    if (!uart)
        return -1;

    frosted_mutex_lock(uart->mutex);
    usart_disable_rx_interrupt(uart->base);
    len_available =  cirbuf_bytesinuse(uart->inbuf);
    if (len_available <= 0) {
        uart->pid = scheduler_get_cur_pid();
        task_suspend();
        out = SYS_CALL_AGAIN;
        goto again;
    }

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(uart->inbuf, ptr) != 0)
            break;
        ptr++;
    }
    uart->pid = 0;

again:
    usart_enable_rx_interrupt(uart->base);
    frosted_mutex_unlock(uart->mutex);
    return out;
}


static int devuart_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct dev_uart *uart = uart_check_fd(fd);
    if (!uart)
        return -1;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
    if ((events == POLLIN) && usart_is_recv_ready(uart->base)) {
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


static int uart_fno_init(struct fnode *dev, uint32_t n)
{
    struct dev_uart *u = &DEV_UART[n];
    char name[6] = "ttyS";
    name[4] = n + '0';

    if (u->base == 0)
        return -1;

    u->fno = fno_create(&mod_devuart, name, dev);
    u->pid = 0;
    u->mutex = frosted_mutex_init();
    u->inbuf = cirbuf_create(256);
    u->fno->priv = u;
    usart_enable_rx_interrupt(u->base);
    nvic_enable_irq(u->irq);
    return 0;

}
                        

void devuart_init(struct fnode *dev)
{
    int i;
    mod_devuart.family = FAMILY_FILE;
    mod_devuart.ops.open = devuart_open;
    mod_devuart.ops.read = devuart_read; 
    mod_devuart.ops.poll = devuart_poll;
    mod_devuart.ops.write = devuart_write;
    
    for (i = 0; i < 4; i++) 
        uart_fno_init(dev, i);

    register_module(&mod_devuart);
}

