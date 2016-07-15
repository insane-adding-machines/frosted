#ifndef INC_SDIO
#define INC_SDIO
#include "frosted.h"
#include "gpio.h"

struct sdio_config {
    uint32_t *rcc_reg;
    uint32_t rcc_en;
    struct gpio_config pio_dat0, pio_dat1, pio_dat2, pio_dat3;
    struct gpio_config pio_clk, pio_cmd;
    
    int card_detect_supported;
    struct gpio_config pio_cd;
};

#ifdef CONFIG_SDIO
int sdio_init(struct sdio_config *conf);
#else
#define sdio_init(x) (-ENOENT)
#endif

#endif
