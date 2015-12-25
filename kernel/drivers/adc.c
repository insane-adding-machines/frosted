#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "adc.h"
#ifdef STM32F4
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#endif


struct dev_adc{
    struct device * dev;
    uint32_t base;
    uint32_t irq;
    uint8_t channel_array[NUM_ADC_CHANNELS];
    uint8_t num_channels;
    int conversion_done;
    uint16_t samples[NUM_ADC_CHANNELS];
};

#define MAX_ADCS 1

static struct dev_adc DEV_ADC[MAX_ADCS];

static int devadc_read(struct fnode * fno, void *buf, unsigned int len);

static struct module mod_devadc = {
    .family = FAMILY_FILE,
    .name = "adc",
    .ops.open = device_open,
    .ops.read = devadc_read,
};

void dma2_stream0_isr()
{
    struct dev_adc * adc = &DEV_ADC[0];

    if((DMA2_LISR & DMA_LISR_TCIF0) != 0)
    {
        dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_LISR_TCIF0);
        adc_disable_dma(adc->base);
        adc->conversion_done = 1;
        if (adc->dev->pid > 0)
            task_resume(adc->dev->pid);
    }
}

static int devadc_read(struct fnode *fno, void *buf, unsigned int len)
{
    int i;
    struct dev_adc *adc;

    if (len <= 0)
        return len;

    adc = (struct dev_adc *)FNO_MOD_PRIV(fno, &mod_devadc);
    if (!adc)
        return -1;

    frosted_mutex_lock(adc->dev->mutex);

    if (adc->conversion_done == 0) 
    {
        adc_enable_dma(adc->base);
        adc_start_conversion_regular(adc->base);
        adc->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(adc->dev->mutex);
        return  SYS_CALL_AGAIN;
    }

    if(len > (adc->num_channels * 2))
    {
        len = adc->num_channels * 2;
    }

    memcpy(buf, adc->samples, len);
    adc->conversion_done = 0;
    frosted_mutex_unlock(adc->dev->mutex);
    return len;
}

static void adc_fno_init(struct fnode *dev, uint32_t n, const struct adc_addr * addr)
{
    struct dev_adc *a = &DEV_ADC[n];
    a->dev = device_fno_init(&mod_devadc, addr->name, dev, FL_RDONLY, a);
    a->base = addr->base;
    memcpy(a->channel_array, addr->channel_array, NUM_ADC_CHANNELS);
    a->num_channels = addr->num_channels;
    a->conversion_done = 0;
}

void adc_init(struct fnode * dev,  const struct adc_addr adc_addrs[], int num_adcs)
{
    int i;
    for (i = 0; i < num_adcs; i++) 
    {
        if (adc_addrs[i].base == 0)
            continue;

        adc_fno_init(dev, i, &adc_addrs[i]);
        CLOCK_ENABLE(adc_addrs[i].rcc);
        CLOCK_ENABLE(adc_addrs[i].dma_rcc);

        dma_stream_reset(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        dma_enable_transfer_complete_interrupt(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        dma_set_transfer_mode(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
        dma_set_priority(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, DMA_SxCR_PL_HIGH);
        dma_enable_circular_mode(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);

        dma_set_peripheral_address(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, (uint32_t) &ADC_DR(adc_addrs[i].base)); //   (adc_addrs[i].base) + 0x4C);
        dma_disable_peripheral_increment_mode(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        dma_set_peripheral_size(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, DMA_SxCR_PSIZE_16BIT);

        dma_set_memory_address(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, (uint32_t) DEV_ADC[i].samples);
        dma_enable_memory_increment_mode(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        dma_set_memory_size(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, DMA_SxCR_MSIZE_16BIT);

        dma_set_number_of_data(adc_addrs[i].dma_base, adc_addrs[i].dma_stream, adc_addrs[i].num_channels);
        dma_enable_direct_mode(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        dma_set_dma_flow_control(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);
        adc_set_resolution(adc_addrs[i].dma_base, ADC_CR1_RES_12BIT);

        nvic_set_priority(adc_addrs[i].dma_irq, 1);
        nvic_enable_irq(adc_addrs[i].dma_irq);
        dma_enable_stream(adc_addrs[i].dma_base, adc_addrs[i].dma_stream);


        adc_off(adc_addrs[i].base);
        adc_disable_external_trigger_regular(adc_addrs[i].base);
        adc_set_sample_time_on_all_channels(adc_addrs[i].base, ADC_SMPR_SMP_480CYC);
        adc_set_regular_sequence(adc_addrs[i].base, adc_addrs[i].num_channels, adc_addrs[i].channel_array);
        adc_enable_scan_mode(adc_addrs[i].base);
        adc_set_single_conversion_mode(adc_addrs[i].base);
        adc_eoc_after_each(adc_addrs[i].base);
        adc_set_dma_terminate(adc_addrs[i].base);
        adc_set_right_aligned(adc_addrs[i].base);
        adc_power_on(adc_addrs[i].base);
    }
}









