#ifndef INC_LSM303DLHC
#define INC_LSM303DLHC

struct lsm303dlhc_addr {
    const char * name;
    const char * i2c_name;
    uint32_t pio1_base;
    uint32_t pio2_base;
    uint32_t drdy_base;
    uint32_t pio1_pin;
    uint32_t pio2_pin;
    uint32_t drdy_pin;
    uint8_t address;
    uint8_t drdy_address;
};

void lsm303dlhc_init(struct fnode *dev, const struct lsm303dlhc_addr lsm303dlhc_addr);

#endif

