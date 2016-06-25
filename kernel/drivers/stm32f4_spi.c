/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors:
 *
 */
 
#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "stm32f4_dma.h"
#include "stm32f4_spi.h"
#include <libopencm3/cm3/nvic.h>

#ifdef LM3S
#   include "libopencm3/lm3s/spi.h"
#   define CLOCK_ENABLE(C)
#endif
#ifdef STM32F4
#   include <libopencm3/stm32/rcc.h>
#   include <libopencm3/stm32/dma.h>
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
    spi_completion completion_fn;
    void * completion_arg;
    const struct dma_setup * tx_dma_setup;
    const struct dma_setup * rx_dma_setup;
};

#define MAX_SPIS 6

static struct dev_spi DEV_SPI[MAX_SPIS];

static struct module mod_devspi = {
    .family = FAMILY_FILE,
    .name = "spi",
    .ops.open = device_open,
};

static void spi1_rx_dma_complete(struct dev_spi *spi)
{
    dma_disable_transfer_complete_interrupt(spi->rx_dma_setup->base, spi->rx_dma_setup->stream);
    spi_disable(spi->base);
    tasklet_add(spi->completion_fn, spi->completion_arg);
    mutex_unlock(spi->dev->mutex);
}


#ifdef CONFIG_SPI_1
void dma2_stream2_isr()
{
    dma_clear_interrupt_flags(DMA2, DMA_STREAM2, DMA_LISR_TCIF0);
    spi1_rx_dma_complete(&DEV_SPI[0]);
}
#endif

int devspi_xfer(struct fnode *fno, spi_completion completion_fn, void * completion_arg, const char *obuf, char *ibuf, unsigned int len)
{
    struct dev_spi *spi;

    if (len <= 0)
        return len;

    spi = (struct dev_spi *)FNO_MOD_PRIV(fno, &mod_devspi);
    if (spi == NULL)
        return -1;

    mutex_lock(spi->dev->mutex);

    spi->completion_fn = completion_fn;
    spi->completion_arg = completion_arg;

    init_dma(spi->tx_dma_setup, (uint32_t)obuf, len);
    init_dma(spi->rx_dma_setup, (uint32_t)ibuf, len);

    dma_enable_transfer_complete_interrupt(spi->rx_dma_setup->base, spi->rx_dma_setup->stream);
    nvic_set_priority(spi->rx_dma_setup->base, 1);
    nvic_enable_irq(spi->rx_dma_setup->irq);

    spi_enable(spi->base);

    spi_enable_rx_dma(spi->base);
    spi_enable_tx_dma(spi->base);

    return len;
}


static void spi_fno_init(struct fnode *dev, uint32_t n, const struct spi_addr * addr)
{
    struct dev_spi *s = &DEV_SPI[n];
    s->dev = device_fno_init(&mod_devspi, addr->name, dev, FL_RDWR, s);
    s->base = addr->base;

    s->tx_dma_setup = &addr->tx_dma;
    s->rx_dma_setup = &addr->rx_dma;
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
        CLOCK_ENABLE(spi_addrs[i].dma_rcc);

        spi_disable(spi_addrs[i].base);

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
    }
    register_module(&mod_devspi);
}
