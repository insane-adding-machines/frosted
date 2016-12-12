#ifndef INC_ETH
#define INC_ETH
#include "frosted.h"
#include "gpio.h"

struct eth_config {
    const struct gpio_config *pio_mii;
    const unsigned int n_pio_mii;
    const struct gpio_config pio_phy_reset;
    const int has_phy_reset;
};

#ifdef CONFIG_DEVETH
int ethernet_init(const struct eth_config *conf);
int pico_eth_start(void);
#else
#  define ethernet_init(x) ((-ENOENT))
#  define pico_eth_start() ((-ENOENT))
#endif

#endif
