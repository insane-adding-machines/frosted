#ifndef INC_GPIO
#define INC_GPIO

struct gpio_addr {
    uint32_t port;
    uint32_t n;
};

struct module * devgpio_init(struct fnode *dev);

#endif
