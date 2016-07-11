#ifndef FRAND_H_
#define FRAND_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "frosted.h"

struct frand_ops;

struct frand_info {
        struct frand_ops *frandops;
        struct device *dev;             /* This is this frand device */
};

struct frand_ops {
        /* open/release and usage marking */
        int (*frand_open)(struct frand_info *info);
};

/* For setting up the generator and feeding entropy */
/* low-level drivers must call this register function first */
int register_frand(struct frand_info *frand_info);

/* kernel init */
void frand_init(struct fnode *dev);

#endif /* FRAND_H_ */

