#ifndef INC_GPIO
#define INC_GPIO

struct gpio_addr {
    uint32_t base;
    uint32_t pin;
    uint32_t mode;
    uint8_t pullupdown;
    uint8_t speed;
    uint8_t optype;
    uint8_t af;
    uint8_t exti;
    uint8_t trigger;
    const char* name;
};

void gpio_init(struct fnode * dev,  const struct gpio_addr gpio_addrs[], int num_gpios);

#endif
