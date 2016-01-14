#ifndef INC_I2C
#define INC_I2C


struct i2c_addr {
    uint32_t base;
    uint32_t ev_irq;
    uint32_t er_irq;
    uint32_t rcc;

    const char * name;

    uint32_t clock_speed;
    uint32_t fast_mode;
    uint32_t rise_time;
    uint32_t bus_clk_frequency;
    uint32_t clock_f;
    uint32_t dma_base;
    uint32_t dma_rcc;
    uint32_t tx_dma_stream;
    uint32_t tx_dma_irq;
    uint32_t rx_dma_stream;
    uint32_t rx_dma_irq;
};

typedef void (* i2c_completion)(void * arg);

void i2c_init(struct fnode *dev, const struct i2c_addr i2c_addrs[], int num_i2cs);
int i2c_read(struct fnode *fno, i2c_completion completion_fn, void * completion_arg, uint8_t addr, uint8_t  register, uint8_t *buf, uint32_t len);
int i2c_write(struct fnode *fno, i2c_completion completion_fn, void * completion_arg, uint8_t addr, uint8_t  register, const uint8_t *buf, uint32_t len);


#endif

