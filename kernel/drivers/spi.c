#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "spi.h"

#ifdef LM3S
#   include "libopencm3/lm3s/spi.h"
#   define CLOCK_ENABLE(C) 
#endif
#ifdef STM32F4
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
    uint32_t dma_base;
    uint32_t tx_dma_stream;
    uint32_t rx_dma_stream;
    uint32_t rx_dma_irq;
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
    dma_disable_transfer_complete_interrupt(spi->dma_base, spi->rx_dma_stream);
    spi_disable(spi->base);
    tasklet_add(spi->completion_fn, spi->completion_arg);
    frosted_mutex_unlock(spi->dev->mutex);
}


#ifdef CONFIG_SPI_1
void dma2_stream2_isr()
{
    dma_clear_interrupt_flags(DMA2, DMA_STREAM2, DMA_LISR_TCIF0);
    spi1_rx_dma_complete(&DEV_SPI[0]);  
}
#endif



static void spi_init_dma(uint32_t base, uint32_t dma, uint32_t stream, uint32_t dirn, uint32_t prio, uint32_t channel)
{
    dma_stream_reset(dma, stream);

    dma_set_transfer_mode(dma, stream, dirn);
    dma_set_priority(dma, stream, prio);
    
    dma_set_peripheral_address(dma, stream, (uint32_t) &SPI_DR(base));
    dma_disable_peripheral_increment_mode(dma, stream);
    dma_set_peripheral_size(dma, stream, DMA_SxCR_PSIZE_8BIT);
    
    dma_enable_memory_increment_mode(dma, stream);
    dma_set_memory_size(dma, stream, DMA_SxCR_MSIZE_8BIT);
    
    dma_enable_direct_mode(dma, stream);
    dma_set_dma_flow_control(dma, stream);

    dma_channel_select(dma,stream,channel);
}

int devspi_xfer(struct fnode *fno, spi_completion completion_fn, void * completion_arg, const char *obuf, char *ibuf, unsigned int len)
{
    struct dev_spi *spi;
    
    if (len <= 0)
        return len;
    
    spi = (struct dev_spi *)FNO_MOD_PRIV(fno, &mod_devspi);
    if (spi == NULL)
        return -1;
    
    frosted_mutex_lock(spi->dev->mutex);

    spi->completion_fn = completion_fn;
    spi->completion_arg = completion_arg;

    spi_init_dma(spi->base, spi->dma_base, spi->tx_dma_stream, DMA_SxCR_DIR_MEM_TO_PERIPHERAL, DMA_SxCR_PL_MEDIUM, DMA_SxCR_CHSEL_3);
    dma_set_memory_address(spi->dma_base, spi->tx_dma_stream, (uint32_t)obuf);
    dma_set_number_of_data(spi->dma_base, spi->tx_dma_stream, len);
    dma_enable_stream(spi->dma_base, spi->tx_dma_stream);

    spi_init_dma(spi->base,spi->dma_base,spi->rx_dma_stream,DMA_SxCR_DIR_PERIPHERAL_TO_MEM, DMA_SxCR_PL_VERY_HIGH, DMA_SxCR_CHSEL_3);
    dma_enable_transfer_complete_interrupt(spi->dma_base, spi->rx_dma_stream);
    nvic_set_priority(spi->rx_dma_irq, 1);
    nvic_enable_irq(spi->rx_dma_irq);

    dma_set_memory_address(spi->dma_base, spi->rx_dma_stream, (uint32_t)ibuf);
    dma_set_number_of_data(spi->dma_base, spi->rx_dma_stream, len);
    dma_enable_stream(spi->dma_base, spi->rx_dma_stream);

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
    s->dma_base = addr->dma_base;
    s->tx_dma_stream = addr->tx_dma_stream;
    s->rx_dma_stream = addr->rx_dma_stream;
    s->rx_dma_irq = addr->rx_dma_irq;
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


