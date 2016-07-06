#ifndef INC_RNG
#define INC_RNG

struct rng_addr {
    uint8_t devidx;
    uint32_t base;
    uint32_t irq;
    uint32_t rcc;
    const char * name;
};


void rng_init(struct fnode *dev, const struct rng_addr rng_addrs[], int num_rng);

#endif

