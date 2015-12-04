#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "l3gd20.h"

struct dev_l3gd20 {
    struct device * dev;
    uint32_t irq;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
};

#define MAX_L3GD20S 1 

static struct dev_l3gd20 DEV_L3GD20S[MAX_L3GD20S];

static int devl3gd20_read(int fd, void *buf, unsigned int len);
static int devl3gd20_poll(int fd, uint16_t events, uint16_t *revents);
static int devl3gd20_write(int fd, const void *buf, unsigned int len);


static struct module mod_devl3gd20 = {
    .family = FAMILY_FILE,
    .name = "l3gd20",
    .ops.open = device_open,
    .ops.read = devl3gd20_read, 
    .ops.poll = devl3gd20_poll,
    .ops.write = devl3gd20_write,
};

static int devl3gd20_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    const struct dev_l3gd20 *l3gd20;

    l3gd20 = device_check_fd(fd, &mod_devl3gd20);
    if (!l3gd20)
        return -1;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
#if 0
    if (uart->w_start == NULL) {
        uart->w_start = (uint8_t *)buf;
        uart->w_end = ((uint8_t *)buf) + len;

    } else {
        /* previous transmit not finished, do not update w_start */
    }
#endif
    frosted_mutex_lock(l3gd20->dev->mutex);
#if 0
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
#endif
    return len;
}


static int devl3gd20_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct hal_iodev *dev;
    const struct dev_l3gd20 *l3gd20;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    l3gd20 = device_check_fd(fd, &mod_devl3gd20);
    if (!l3gd20)
        return -1;

    frosted_mutex_lock(l3gd20->dev->mutex);
#if 0
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
#endif
again:
//    usart_enable_rx_interrupt(uart->base);
    frosted_mutex_unlock(l3gd20->dev->mutex);
    return out;
}

static int devl3gd20_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    const struct dev_l3gd20 *l3gd20 = device_check_fd(fd, &mod_devl3gd20);
    if (!l3gd20)
        return -1;
    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
//    if ((events == POLLIN) && usart_is_recv_ready(l3gd20->base)) {
//        *revents |= POLLIN;
//        ret = 1;
//    }
    return ret;
}

static void l3gd20_fno_init(struct fnode *dev, uint32_t n, const struct l3gd20_addr * addr)
{
    struct dev_l3gd20 *l = &DEV_L3GD20S[n];
    l->dev = device_fno_init(&mod_devl3gd20, addr->name, dev, FL_RDWR, l);
}


void l3gd20_init(struct fnode * dev, const struct l3gd20_addr l3gd20_addrs[], int num_l3gd20s)
{
    int i;
    for (i = 0; i < num_l3gd20s; i++) 
    {
        l3gd20_fno_init(dev, i, &l3gd20_addrs[i]);
//        l3gd20_enable(l3gd20_addrs[i].base);
    }
    register_module(&mod_devl3gd20);
}

