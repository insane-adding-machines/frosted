#ifndef LOWPOWER_H_INC
#define LOWPOWER_H_INC
#include <stdint.h>

#ifndef CONFIG_LOWPOWER
#   define lowpower_sleep(x) (-1)
#   define lowpower_resume() do{}while(0)
#else
    int lowpower_sleep(uint32_t interval);
    void lowpower_resume(void);
#endif
#endif
