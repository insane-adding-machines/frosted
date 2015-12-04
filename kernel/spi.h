#ifndef INC_SPI
#define INC_SPI

#define MAX_SPI_PINS 3

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
};


void spi_init(struct fnode *dev, const struct spi_addr spi_addrs[], int num_spi);

#endif
