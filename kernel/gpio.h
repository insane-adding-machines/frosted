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

typedef void (* exti_callback)(void * arg);

void gpio_init(struct fnode * dev,  const struct gpio_addr gpio_addrs[], int num_gpios);
void gpio_register_exti_callback(struct fnode *fno, exti_callback callback_fn, void * callback_arg);
#endif
