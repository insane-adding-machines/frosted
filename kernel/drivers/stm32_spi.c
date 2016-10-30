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
#include <unicore-mx/cm3/nvic.h>

#include <unicore-mx/stm32/dma.h>
#include "unicore-mx/stm32/spi.h"
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/stm32/gpio.h>
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "dma.h"
#include "spi.h"
#include "locks.h"


/* Dummy module, for gpio claiming only. */
static struct module mod_spi = {
    .family = FAMILY_DEV,
    .name = "spi"
};

struct dev_spi {
    struct device * dev;
    uint32_t base;
    uint32_t irq;
    void (*isr)(struct spi_slave *sl);
    struct spi_slave *sl;
    const struct dma_config * tx_dma_config;
    const struct dma_config * rx_dma_config;
    mutex_t *mutex;
};

#define MAX_SPIS 4

static struct dev_spi *DEV_SPI[MAX_SPIS] = { };

static void spi_rx_dma_complete(struct spi_slave * sl)
{
    struct dev_spi *spi;
    if (!sl)
        return;
    spi = DEV_SPI[sl->bus];
    if (!spi)
        return;
    if (spi->isr)
        spi->isr(sl);
}

#ifdef CONFIG_SPI_1
void spi_isr(void)
{
}

void dma2_stream2_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM0, DMA_LISR_TCIF0);
    spi_rx_dma_complete(DEV_SPI[1]->sl);
    spi_disable(DEV_SPI[1]->base);
    mutex_unlock(DEV_SPI[1]->mutex);
}

void dma2_stream3_isr()
{
    dma_clear_interrupt_flags(DMA1, DMA_STREAM6, DMA_LISR_TCIF0);
}
#endif

int devspi_xfer(struct spi_slave *sl, const char *obuf, char *ibuf, unsigned int len)
{
    struct dev_spi *spi;
    int i;
    if (len <= 0)
        return len;

    spi = DEV_SPI[sl->bus];
    if (!spi)
        return -EINVAL;

    mutex_lock(spi->mutex);
    spi_reset(spi->base);
    spi->sl = sl;
    //init_dma(spi->tx_dma_config, (uint32_t)obuf, len);
    //init_dma(spi->rx_dma_config, (uint32_t)ibuf, len);

    //dma_enable_transfer_complete_interrupt(spi->rx_dma_config->base, spi->rx_dma_config->stream);
    //nvic_set_priority(spi->rx_dma_config->irq, 1);
    //nvic_enable_irq(spi->rx_dma_config->irq);

    
    spi_enable(spi->base);
    //spi_enable_rx_dma(spi->base);
    //spi_enable_tx_dma(spi->base);

    for(i = 0; i < len; i++) {
        spi_send(spi->base, (uint16_t)obuf[i]);
    }

    return len;
}

int devspi_create(const struct spi_config *conf)
{
    struct dev_spi *spi = NULL;

    if (!conf)
        return -EINVAL;
    if (conf->base == 0)
        return -EINVAL;

    if ((conf->idx < 0) || (conf->idx > MAX_SPIS))
        return -EINVAL;

    spi = kalloc(sizeof(struct dev_spi));
    if (!spi)
        return -ENOMEM;

    /* Claim pins for SCK/MOSI/MISO */
    gpio_create(&mod_spi, &conf->pio_sck);
    gpio_create(&mod_spi, &conf->pio_mosi);
    gpio_create(&mod_spi, &conf->pio_miso);

    /* Erase spi content */
    memset(spi, 0, sizeof(struct dev_spi));

    /* Enable clocks */
    rcc_periph_clock_enable(conf->rcc);
    rcc_periph_clock_enable(conf->dma_rcc);

    /* Startup routine */
    //spi_disable(conf->base);

    /**********************************/
	/* reset SPI1 */
	spi_reset(SPI1);
	/* init SPI1 master */
	spi_init_master(SPI1,
					SPI_CR1_BAUDRATE_FPCLK_DIV_64,
					SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
					SPI_CR1_CPHA_CLK_TRANSITION_1,
					SPI_CR1_DFF_8BIT,
					SPI_CR1_MSBFIRST);
	/* enable SPI1 first */
	spi_enable(SPI1);
    /**********************************/

#if 0
    spi_set_master_mode(conf->base);
    spi_set_baudrate_prescaler(conf->base, SPI_CR1_BR_FPCLK_DIV_256); /* TODO: Calculate prescaler from baudrate */
    if(conf->polarity == 0) 
        spi_set_clock_polarity_0(conf->base);
    else                    
        spi_set_clock_polarity_1(conf->base);
    if(conf->phase == 0) spi_set_clock_phase_0(conf->base);
    else
        spi_set_clock_phase_1(conf->base);
    if(conf->rx_only == 0)      
        spi_set_full_duplex_mode(conf->base);
    else
        spi_set_receive_only_mode(conf->base);
    if(conf->bidir_mode == 0)      
        spi_set_unidirectional_mode(conf->base);
    else
        spi_set_bidirectional_mode(conf->base);
    if(conf->dff_16) 
        spi_set_dff_16bit(conf->base);
    else
        spi_set_dff_8bit(conf->base);
    if(conf->enable_software_slave_management) 
        spi_enable_software_slave_management(conf->base);
    else
        spi_disable_software_slave_management(conf->base);
    if(conf->send_msb_first) 
        spi_send_msb_first(conf->base);
    else
        spi_send_lsb_first(conf->base);
    spi_set_nss_high(conf->base);
#endif

    /* Set up device struct */
    spi->base = conf->base;
    spi->irq = conf->irq;
    //spi->tx_dma_config = &conf->tx_dma;
    //spi->rx_dma_config = &conf->rx_dma;
    spi->mutex = mutex_init();

    /* Store address in the DEV_SPI array. */
    DEV_SPI[conf->idx] = spi;

    /* Enable interrupts */
    //nvic_set_priority(conf->irq, 1);
    //nvic_enable_irq(conf->irq);
    return 0;
}
