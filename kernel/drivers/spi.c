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

static int devspi_write(struct fnode *fno, const void *buf, unsigned int len);
static int devspi_read(struct fnode *fno, void *buf, unsigned int len);

static const struct module mod_devspi = {
    .family = FAMILY_FILE,
    .name = "spi",
    .ops.open = device_open,
    .ops.read = devspi_read,
    .ops.write = devspi_write,
};

void spi_isr(struct dev_spi *spi)
{
    /* TX interrupt */
    if (SPI_SR(spi->base) & SPI_SR_TXE) {
        spi_disable_tx_buffer_empty_interrupt(spi->base);
        frosted_mutex_lock(spi->dev->mutex);
        /* Are there bytes left to be written? */
        if (cirbuf_bytesinuse(spi->outbuf))
        {
            uint8_t outbyte;
            cirbuf_readbyte(spi->outbuf, &outbyte);
            usart_send(spi->base, (uint16_t)(outbyte));
        } else {
            spi_disable_tx_buffer_empty_interrupt(spi->base);
        }
        frosted_mutex_unlock(spi->dev->mutex);
    }

    /* RX interrupt, data available */
    if (SPI_SR(spi->base) & SPI_SR_RXNE) {
        /* read data into circular buffer */
        cirbuf_writebyte(spi->inbuf, spi_read(spi->base));
    }

    /* If a process is attached, resume the process */
    if (spi->dev->pid > 0) 
        task_resume(spi->dev->pid);
}

#ifdef CONFIG_SPI_1
void spi1_isr(void)
{
    uart_isr(&DEV_SPI[0]);  /* NOTE the -1, spi numbering starts at 1*/
}
#endif

static int devspi_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    struct dev_spi *spi;

    spi = (struct dev_spi *)FNO_MOD_PRIV(fno, &mod_devspi);
    if (!spi)
        return -1;
    if (len <= 0)
        return len;

    if (spi->w_start == NULL) {
        spi->w_start = (uint8_t *)buf;
        spi->w_end = ((uint8_t *)buf) + len;

    } else {
        /* previous transmit not finished, do not update w_start */
    }

    frosted_mutex_lock(spi->dev->mutex);
    spi_enable_tx_buffer_empty_interrupt(spi->base);

    /* write to circular output buffer */
    spi->w_start += cirbuf_writebytes(spi->outbuf, spi->w_start, spi->w_end - spi->w_start);
    if (!(SPI_SR(spi->base) & SPI_SR_TXE)) {
        uint8_t c;
        /* Doesn't block because of test above */
        cirbuf_readbyte(spi->outbuf, &c);
        spi_send(spi->base, c);
    }

    if (cirbuf_bytesinuse(spi->outbuf) == 0) {
        frosted_mutex_unlock(spi->dev->mutex);
        spi_disable_tx_buffer_empty_interrupt(spi->base);
        spi->w_start = NULL;
        spi->w_end = NULL;
        return len;
    }


    if (spi->w_start < spi->w_end)
    {
        spi->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(spi->dev->mutex);
        return SYS_CALL_AGAIN;
    }

    frosted_mutex_unlock(spi->dev->mutex);
    spi->w_start = NULL;
    spi->w_end = NULL;

    return len;
}


static int devspi_read(struct fnode *fno, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct dev_spi *spi;

    if (len <= 0)
        return len;

    spi = (struct dev_spi *)FNO_MOD_PRIV(fno, &mod_devspi);
    if (!spi)
        return -1;

    frosted_mutex_lock(spi->dev->mutex);
    spi_disable_rx_buffer_not_empty_interrupt(spi->base);
    len_available =  cirbuf_bytesinuse(spi->inbuf);
    if (len_available <= 0) {
        spi->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(spi->dev->mutex);
        out = SYS_CALL_AGAIN;
        goto again;
    }

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(spi->inbuf, ptr) != 0)
            break;
        ptr++;
    }
again:
//    usart_enable_rx_interrupt(uart->base);
    frosted_mutex_unlock(spi->dev->mutex);
    return out;
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


