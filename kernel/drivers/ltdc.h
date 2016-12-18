#ifndef INC_LTDC
#define INC_LTDC

#ifdef CONFIG_LTDC
int ltdc_init(void);
#else
#  define ltdc_init() ((-ENOENT))
#endif
#endif
