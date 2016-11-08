#ifndef INC_DSP
#define INC_DSP

#ifdef CONFIG_DSP
int dsp_init(void);
#else
#  define dsp_init() ((-ENOENT))
#endif

#endif
