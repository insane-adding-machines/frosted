#ifndef INC_ADC
#define INC_ADC

#define NUM_ADC_CHANNELS    16

struct adc_addr {
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    const char * name;
    uint8_t channel_array[NUM_ADC_CHANNELS];
    uint8_t num_channels;

    struct dma_config dma;
    uint32_t dma_rcc;
};

void adc_init(struct fnode *dev, const struct adc_addr adc_addrs[], int num_adc);

#endif

