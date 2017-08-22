#ifndef SPI_MMC_INC
#define SPI_MMC_INC
#   ifdef CONFIG_DEVSPI_MMC
        int spi_mmc_init(uint8_t bus);
#   else
#       define spi_mmc_init(...) (0)
#   endif
#endif
