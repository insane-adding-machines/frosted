#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include <stdint.h>
#include "frosted.h"

struct fb_addr {
    uint8_t devidx;
    uint32_t base;
};

void fb_init(struct fnode * dev);

#endif
