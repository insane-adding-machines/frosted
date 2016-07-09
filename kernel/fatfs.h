#include "frosted.h"

#ifndef FATFS_INC
#define FATFS_INC


#ifdef CONFIG_FATFS

int fatfs_init(void);

#else
#define fatfs_init() ((-ENOENT))

#endif


#endif
