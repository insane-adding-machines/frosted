#ifndef INC_GPIO
#define INC_GPIO
#include "frosted.h"
#include "sys/frosted-io.h"



#if defined(STM32F4) || defined(STM32F7)
#include <unicore-mx/stm32/gpio.h>
#endif


struct gpio_config {
    uint32_t base;
    uint32_t pin;
    uint32_t mode;
    uint8_t pullupdown;
    uint8_t speed;
    uint8_t optype;
    uint8_t af;
    uint32_t trigger;
    const char* name;
};

#define GPIO_FL_PROTECTED (0x10)
#define SET_TRIGGER_WAITING(x,t) x->flags|=t
#define RESET_TRIGGER_WAITING(x) x->flags&=0xFFFFFFF0
#define TRIGGER_WAITING(x) (((x->flags & 0x0F) == GPIO_TRIGGER_RAISE)?1:0)

#define IS_PROTECTED(x) ((x->flags & GPIO_FL_PROTECTED) == GPIO_FL_PROTECTED)

struct dev_gpio {
    struct device *dev;
    struct module *owner;  /* Module that registered the gpio */
    uint32_t mode;
    uint32_t af;
    uint32_t base;
    uint32_t pin;
    uint8_t trigger;
    int exti_idx;
    unsigned int flags;
    uint32_t optype;
    uint32_t speed;
    uint8_t pullupdown;
    struct dev_gpio *next;
};

extern struct dev_gpio *Gpio_list;

int gpio_init(void);
int gpio_list_len(void);
int gpio_create(struct module *mod, const struct gpio_config *gpio_config);
#endif
