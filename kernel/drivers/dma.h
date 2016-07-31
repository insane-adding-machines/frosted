#ifndef INC_DMA
#define INC_DMA

struct dma_config {
    uint32_t base;
    uint8_t stream;
    uint32_t channel;
    uint32_t psize;
    uint32_t msize;
    uint32_t dirn;
    uint32_t prio;
    uint32_t paddr;
    uint32_t irq;
};

void init_dma(const struct dma_config * dma, uint32_t ptr, uint32_t len);

#endif

