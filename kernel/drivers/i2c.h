#ifndef INC_I2C
#define INC_I2C
#include "dma.h"
#include "gpio.h"


struct i2c_config {
    int idx;
    uint32_t base;
    uint32_t ev_irq;
    uint32_t er_irq;
    uint32_t rcc;
    uint32_t clock_speed;
    uint32_t fast_mode;
    uint32_t rise_time;
    uint32_t bus_clk_frequency;
    uint32_t clock_f;
    uint32_t dma_rcc;
    struct dma_config tx_dma;
    struct dma_config rx_dma;
    struct gpio_config pio_scl;
    struct gpio_config pio_sda;
};

struct i2c_slave {
    int     bus;
    uint8_t address;
    void (*isr_tx)(struct i2c_slave *);
    void (*isr_rx)(struct i2c_slave *);
    void *priv;
};

int i2c_create(const struct i2c_config *i2c_config);
int i2c_init_read(struct i2c_slave *sl, uint8_t reg, uint8_t *buf, uint32_t len);
int i2c_init_write(struct i2c_slave *sl, uint8_t reg, const uint8_t *buf, uint32_t len);
int i2c_kthread_read(struct i2c_slave *sl, uint8_t reg, uint8_t *buf, uint32_t len);
int i2c_kthread_write(struct i2c_slave *sl, uint8_t reg, const uint8_t *buf, uint32_t len);

#endif

