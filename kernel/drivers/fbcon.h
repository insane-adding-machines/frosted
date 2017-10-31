#ifndef FBCON_INCLUDED
#define FBCON_INCLUDED

#include <stdint.h>
#include "frosted.h"

#ifdef CONFIG_DEVFBCON
/* kernel init */
int fbcon_init(uint32_t cols, uint32_t rows);
#else
#  define fbcon_init(...) ((-ENOENT))
#endif

#endif
