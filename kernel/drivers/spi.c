#include "frosted.h"
#include "device.h"
#include <stdint.h>
//#include "spi_dev.h"
#include "spi.h"

#ifdef LM3S
#   include "libopencm3/lm3s/spi.h"
#   define CLOCK_ENABLE(C) 
#endif
#ifdef STM32F4
#   include "libopencm3/stm32/spi.h"
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#endif
#ifdef LPC17XX
#   include "libopencm3/lpc17xx/spi.h"
#   define CLOCK_ENABLE(C) 
#endif

struct dev_spi {
    struct device * dev;
    uint32_t base;
    uint32_t irq;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
};

#define MAX_SPIS 6

static struct dev_spi DEV_SPI[MAX_SPIS];

static int devspi_write(int fd, const void *buf, unsigned int len);
static int devspi_read(int fd, void *buf, unsigned int len);
static int devspi_poll(int fd, uint16_t events, uint16_t *revents);

static struct module mod_devspi = {
    .family = FAMILY_FILE,
    .name = "spi",
    .ops.open = device_open,
    .ops.read = devspi_read,
    .ops.write = devspi_write,
    .ops.poll = devspi_poll,
};

void spi_isr(struct dev_spi *spi)
{
}

#ifdef CONFIG_SPI_1
void spi1_isr(void)
{
    uart_isr(&DEV_SPI[0]);  /* NOTE the -1, spi numbering starts at 1*/
}
#endif

static int devspi_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    struct dev_spi *spi;

    spi = (struct dev_spi *)device_check_fd(fd, &mod_devspi);
    if (!spi)
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
    frosted_mutex_lock(spi->dev->mutex);
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


static int devspi_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct dev_spi *spi;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    spi = (struct dev_spi *)device_check_fd(fd, &mod_devspi);
    if (!spi)
        return -1;

    frosted_mutex_lock(spi->dev->mutex);
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
    frosted_mutex_unlock(spi->dev->mutex);
    return out;
}


static int devspi_poll(int fd, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    struct dev_spi *spi;

    spi = (struct dev_spi *)device_check_fd(fd, &mod_devspi);
    if (!spi)
        return -1;

    *revents = 0;
    if (events & POLLOUT) {
        *revents |= POLLOUT;
        ret = 1; /* TODO: implement interrupt for write events */
    }
    if ((events == POLLIN) && usart_is_recv_ready(spi->base)) {
        *revents |= POLLIN;
        ret = 1;
    }
    return ret;
}

static void spi_fno_init(struct fnode *dev, uint32_t n, const struct spi_addr * addr)
{
    struct dev_spi *s = &DEV_SPI[n];
    s->dev = device_fno_init(&mod_devspi, addr->name, dev, FL_RDWR, s);
}

void spi_init(struct fnode * dev, const struct spi_addr spi_addrs[], int num_spis)
{
    int i;

    for (i = 0; i < num_spis; i++) 
    {
        if (spi_addrs[i].base == 0)
            continue;

        spi_fno_init(dev, i, &spi_addrs[i]);
        CLOCK_ENABLE(spi_addrs[i].rcc);
        spi_set_master_mode(spi_addrs[i].base);
        spi_set_baudrate_prescaler(spi_addrs[i].base, spi_addrs[i].baudrate_prescaler);
        if(spi_addrs[i].clock_pol == 0) spi_set_clock_polarity_0(spi_addrs[i].base);
        else                                            spi_set_clock_polarity_1(spi_addrs[i].base);
        if(spi_addrs[i].clock_phase == 0) spi_set_clock_phase_0(spi_addrs[i].base);
        else                                            spi_set_clock_phase_1(spi_addrs[i].base);
        if(spi_addrs[i].rx_only == 0)      spi_set_full_duplex_mode(spi_addrs[i].base);
        else                                            spi_set_receive_only_mode(spi_addrs[i].base);
        if(spi_addrs[i].bidir_mode == 0)      spi_set_unidirectional_mode(spi_addrs[i].base);
        else                                            spi_set_bidirectional_mode(spi_addrs[i].base);
        if(spi_addrs[i].dff_16) spi_set_dff_16bit(spi_addrs[i].base);
        else                                spi_set_dff_8bit(spi_addrs[i].base);
        if(spi_addrs[i].enable_software_slave_management) spi_enable_software_slave_management(spi_addrs[i].base);
        else                                                                                spi_disable_software_slave_management(spi_addrs[i].base);
        if(spi_addrs[i].send_msb_first) spi_send_msb_first(spi_addrs[i].base);
        else                                            spi_send_lsb_first(spi_addrs[i].base);
        spi_set_nss_high(spi_addrs[i].base);

        SPI_I2SCFGR(spi_addrs[i].base) &= ~SPI_I2SCFGR_I2SMOD;
        spi_enable(spi_addrs[i].base);
        nvic_enable_irq(spi_addrs[i].irq);
    }
    register_module(&mod_devspi);
}


