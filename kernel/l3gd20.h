#ifndef INC_L3GD20
#define INC_L3GD20

struct l3gd20_addr {
    const char * name;
    const char * spi_name;
    const char * spi_cs_name;
    const char * int_1_name;
    const char * int_2_name;
    /* TBD, speed, spi mode */
};

void l3gd20_init(struct fnode *dev, const struct l3gd20_addr l3gd20_addrs[], int num_l3gd20);

#endif

