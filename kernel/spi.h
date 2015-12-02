#ifndef INC_SPI
#define INC_SPI

#include "gpio.h"

/* MISO, MOSI, CLK, N * CS*/
#define MAX_SPI_PINS 3

struct spi_addr {
/*
    uint8_t devidx;
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    uint32_t baudrate;
    uint8_t stop_bits;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t flow;   */
};


void spi_init(struct fnode *dev, const struct spi_addr spi_addrs[], int num_spi);

#endif
