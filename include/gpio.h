#ifndef INC_GPIO
#define INC_GPIO

struct gpio_addr {
    uint32_t port;
    uint32_t rcc;
    uint32_t pin;
    uint32_t mode;
    uint8_t pullupdown;
    uint8_t speed;
    uint8_t optype;
    uint8_t af;
    const char* name;
};

struct module * devgpio_init(struct fnode *dev);

#endif
