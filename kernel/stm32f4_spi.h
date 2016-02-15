#ifndef INC_SPI
#define INC_SPI

struct spi_addr {
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    /* For simplicity we assume all devices on the SPI bus use the same setup,
        this may change later... */
    uint8_t baudrate_prescaler;
    uint8_t clock_pol;
    uint8_t clock_phase;
    uint8_t rx_only;
    uint8_t bidir_mode;
    uint16_t dff_16;
    uint8_t enable_software_slave_management;
    uint8_t send_msb_first;

    const char * name;

    struct dma_setup tx_dma;
    struct dma_setup rx_dma;

//    uint32_t dma_base;
    uint32_t dma_rcc;
//    uint32_t tx_dma_stream;
//    uint32_t rx_dma_stream;
    uint32_t rx_dma_irq;
};

typedef void (* spi_completion)(void * arg);


void spi_init(struct fnode *dev, const struct spi_addr spi_addrs[], int num_spi);

int devspi_xfer(struct fnode *fno, spi_completion completion_fn, void * completion_arg, const char *obuf, char *ibuf, unsigned int len);


#endif
