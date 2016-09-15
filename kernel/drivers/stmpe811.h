#ifndef INC_STMPE811
#define INC_STMPE811

#include "gpio.h"

struct ts_config {
    struct gpio_config gpio;
    uint8_t bus;
};


int stmpe811_init(struct ts_config *ts);

#endif
