#include "frosted.h"
#include <stdint.h>
#include "spi_dev.h"
#include "spi.h"

struct dev_spi {
    uint32_t base;
    uint32_t irq;
    struct fnode *fno;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
    frosted_mutex_t *mutex;
    uint16_t pid;
};

#define MAX_SPIS 6

static struct dev_uart DEV_SPI[MAX_SPIS];

static struct module mod_devspi = {
};

void spi_isr(struct dev_spi *spi)
{
}

#ifdef SPI1
void usart1_isr(void)
{
    uart_isr(&DEV_UART[1]);
}
#endif

static int devspi_write(int fd, const void *buf, unsigned int len)
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

    if (uart->w_start == NULL) {
        uart->w_start = (uint8_t *)buf;
        uart->w_end = ((uint8_t *)buf) + len;

    } else {
        /* previous transmit not finished, do not update w_start */
    }

    frosted_mutex_lock(uart->mutex);
    usart_enable_tx_interrupt(uart->base);

    /* write to circular output buffer */
    uart->w_start += cirbuf_writebytes(uart->outbuf, uart->w_start, uart->w_end - uart->w_start);
    if (usart_is_send_ready(uart->base)) {
        char c;
        cirbuf_readbyte(uart->outbuf, &c);
        usart_send(uart->base, (uint16_t) c);
    }

    if (cirbuf_bytesinuse(uart->outbuf) == 0) {
        frosted_mutex_unlock(uart->mutex);
        usart_disable_tx_interrupt(uart->base);
        uart->w_start = NULL;
        uart->w_end = NULL;
        return len;
    }


    if (uart->w_start < uart->w_end)
    {
        uart->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(uart->mutex);
        return SYS_CALL_AGAIN;
    }

    frosted_mutex_unlock(uart->mutex);
    uart->w_start = NULL;
    uart->w_end = NULL;
    return len;
}


static int devspi_read(int fd, void *buf, unsigned int len)
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
        frosted_mutex_unlock(uart->mutex);
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

again:
    usart_enable_rx_interrupt(uart->base);
    frosted_mutex_unlock(uart->mutex);
    return out;
}


static int devspi_poll(int fd, uint16_t events, uint16_t *revents)
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

static int devspi_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    return task_filedesc_add(f); 
}


static struct module * devspi_init(struct fnode *dev)
{
    mod_devspi.family = FAMILY_FILE;
    strcpy(mod_devspi.name,"spi");
    mod_devspi.ops.open = devspi_open;
    mod_devspi.ops.read = devspi_read; 
    mod_devspi.ops.poll = devspi_poll;
    mod_devspi.ops.write = devspi_write;
    
    return &mod_devspi;
}

void spi_init(struct fnode * dev, const struct spi_addr spi_addrs[], int num_spis)
{
    int i,j;
    struct module * devspi = devspi_init(dev);

    for (i = 0; i < num_spis; i++) 
    {
        CLOCK_ENABLE(uart_addrs[i].rcc);
        uart_fno_init(dev, uart_addrs[i].devidx, &uart_addrs[i]);
        usart_set_baudrate(uart_addrs[i].base, uart_addrs[i].baudrate);
        usart_set_databits(uart_addrs[i].base, uart_addrs[i].data_bits);
        usart_set_stopbits(uart_addrs[i].base, uart_addrs[i].stop_bits);
        usart_set_mode(uart_addrs[i].base, USART_MODE_TX_RX);
        usart_set_parity(uart_addrs[i].base, uart_addrs[i].parity);
        usart_set_flow_control(uart_addrs[i].base, uart_addrs[i].flow);
        /* one day we will do non blocking UART Tx and will need to enable tx interrupt */
        usart_enable_rx_interrupt(uart_addrs[i].base);
        /* Finally enable the USART. */
        usart_enable(uart_addrs[i].base);
    }
    register_module(devspi);
}


