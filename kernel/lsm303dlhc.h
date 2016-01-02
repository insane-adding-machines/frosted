#ifndef INC_LSM303DLHC
#define INC_LSM303DLHC

struct lsm303dlhc_addr {
    const char * name;
    const char * i2c_name;
    const char * int_1_name;
    const char * int_2_name;
    const char * drdy_name;
    uint8_t address;
};
void lsm303dlhc_init(struct fnode *dev, const struct lsm303dlhc_addr lsm303dlhc_addrs[], int num_lsm303dlhc);

#endif

