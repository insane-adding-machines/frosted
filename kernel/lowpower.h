#ifndef LOWPOWER_H_INC
#define LOWPOWER_H_INC
#include <stdint.h>

#ifndef CONFIG_LOWPOWER
#   define lowpower_init() (-1)
#   define lowpower_sleep(x,y) (-1)
#else
    int lowpower_init(void);
    int lowpower_sleep(int stdby, uint32_t interval);
#endif
#endif
