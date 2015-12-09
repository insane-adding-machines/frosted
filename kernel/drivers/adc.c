#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "adc.h"
#ifdef STM32F4
#include <libopencm3/stm32/adc.h>
#   define CLOCK_ENABLE(C)                 rcc_periph_clock_enable(C);
#endif


struct dev_adc{
    struct device * dev;
    uint32_t base;
    uint32_t irq;
    uint8_t channel_array[NUM_ADC_CHANNELS];
    uint8_t num_channels;
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

static int conversion_done = 0;

void adc_isr(void)
{
    struct dev_adc * adc = &DEV_ADC[0];

    if(adc_eoc(adc->base) == 0)
    {
        conversion_done = 1;

        /* If a process is attached, resume the process */
        if (adc->dev->pid > 0) 
            task_resume(adc->dev->pid);
    }
}

static int devadc_read(struct fnode *fno, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct dev_adc *adc;

    if (len <= 0)
        return len;

    adc = (struct dev_adc *)FNO_MOD_PRIV(fno, &mod_devadc);
    if (!adc)
        return -1;

    frosted_mutex_lock(adc->dev->mutex);

    if (conversion_done == 0) {
        adc_set_regular_sequence(adc->base, adc->num_channels, adc->channel_array);
        adc_start_conversion_regular(adc->base);

        adc->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(adc->dev->mutex);
        out = SYS_CALL_AGAIN;
        goto again;
    }
    else
    {
        /* To be improved */
        out = 2;
        *((uint16_t *) buf) = adc_read_regular(adc->base);
        conversion_done = 1;
    }

again:
//    usart_enable_rx_interrupt(uart->base);
    frosted_mutex_unlock(adc->dev->mutex);
    return out;

}

static void adc_fno_init(struct fnode *dev, uint32_t n, const struct adc_addr * addr)
{
    struct dev_adc *a = &DEV_ADC[n];
    a->dev = device_fno_init(&mod_devadc, addr->name, dev, FL_RDONLY, a);
    a->base = addr->base;
    memcpy(a->channel_array, addr->channel_array, NUM_ADC_CHANNELS);
    a->num_channels = addr->num_channels;
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

        adc_off(adc_addrs[i].base);
        adc_disable_scan_mode(adc_addrs[i].base);
        adc_set_sample_time_on_all_channels(adc_addrs[i].base, ADC_SMPR_SMP_3CYC);
        adc_power_on(adc_addrs[i].base);

        nvic_enable_irq(adc_addrs[i].irq);
    }
}









