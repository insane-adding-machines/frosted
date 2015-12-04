#include "frosted.h"
#include "device.h"

int device_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    return task_filedesc_add(f); 
}

const void *device_check_fd(int fd, struct module * mod)
{
    struct fnode *fno;
    fno = task_filedesc_get(fd);
    
    if (!fno)
        return 0;
    if (fd < 0)
        return 0;

    if (fno->owner != mod)
        return 0;
    return fno->priv;
}

struct device *  device_fno_init(struct module * mod, const char * name, struct fnode *node, uint32_t flags, void * priv)
{
    struct device * device = kalloc(sizeof(struct device));
    device->fno = NULL;
    /* Only create a device node if there is a name */
    if(name)
    {
        device->fno =  fno_create(mod, name, node);
        device->fno->priv = priv;
        device->fno->flags |= flags;
    }
    device->pid = 0;
    device->mutex = frosted_mutex_init();
    return device;
}



