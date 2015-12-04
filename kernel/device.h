#ifndef INC_DEVICE
#define INC_DEVICE

#include "frosted.h"

struct device {
    struct fnode *fno;
    frosted_mutex_t * mutex;
    uint16_t pid;
};

int device_open(const char *path, int flags);
const void *device_check_fd(int fd, struct module * mod);
struct device *  device_fno_init(struct module * mod, const char * name, struct fnode *node, uint32_t flags,void * priv);


#endif
