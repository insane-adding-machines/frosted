#ifndef INC_SDRAM
#define INC_SDRAM

#if defined CONFIG_SDRAM
int sdram_init(void);
#else
#define sdram_init() (-ENOENT)
#endif

#endif
