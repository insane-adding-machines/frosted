#ifndef INC_SPI
#define INC_SPI
#include "dma.h"
#include "gpio.h"

struct spi_config {
    int idx;
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    uint32_t baudrate;
    uint8_t polarity;
    uint8_t phase;
    uint8_t rx_only;
    uint8_t bidir_mode;
    uint16_t dff_16;
    uint8_t enable_software_slave_management;
    uint8_t send_msb_first;
    
    /* DMA Config */
    uint32_t dma_rcc;
    struct dma_config tx_dma;
    struct dma_config rx_dma;

    /* Pin muxing */
    struct gpio_config pio_sck;
    struct gpio_config pio_miso;
    struct gpio_config pio_mosi;
    struct gpio_config pio_nss;
};

struct spi_slave {
    uint8_t bus;
    void (*isr)(struct spi_slave *);
    void *priv;
};

int devspi_create(const struct spi_config *spi_config);
int devspi_xfer(struct spi_slave *spi, const char *obuf, char *ibuf, unsigned int len);

#endif

