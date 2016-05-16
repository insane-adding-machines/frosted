#ifndef INC_DEVICE
#define INC_DEVICE

#include "frosted.h"

struct device {
    struct fnode *fno;
    frosted_mutex_t * mutex;
    int pid;
};

int device_open(const char *path, int flags);
struct device *  device_fno_init(struct module * mod, const char * name, struct fnode *node, uint32_t flags,void * priv);


#endif
