
#ifndef INC_TTY_CONSOLE
#define INC_TTY_CONSOLE
#include "frosted.h"
#ifdef CONFIG_DEVTTY_CONSOLE
int tty_console_init(void);
#else
#define tty_console_init() (-ENOENT)
#endif


#endif

