#ifndef RNG_INC
#define RNG_INC
#include "frosted.h"

#ifdef CONFIG_RNG

int rng_init(void);
int rng_create(uint32_t base, uint32_t rcc);

#else

#define rng_init() (-ENOENT)
#define rng_create(...) (-ENOENT)

#endif


#endif
