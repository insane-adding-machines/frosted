#ifndef INC_L3GD20
#define INC_L3GD20

struct l3gd20_addr {
    const char * name;
    const char * spi_name;
    const char * spi_cs_name;
    uint32_t pio1_base;
    uint32_t pio1_pin;
    uint32_t pio2_base;
    uint32_t pio2_pin;

};

void l3gd20_init(struct fnode *dev, const struct l3gd20_addr l3gd20_addr);

#endif

