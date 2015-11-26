#include "frosted.h"
#include <string.h>



static int devnull_read(int fd, void *buf, unsigned int len)
{
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    memset(buf, 0, sizeof(len));
    return (int)len;
}


static int devnull_write(int fd, const void *buf, unsigned int len)
{
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;
    return len;
}

static int devnull_poll(int fd, uint16_t events, uint16_t *revents)
{
    return 1;
}

static int devnull_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    return task_filedesc_add(f);

}


static struct fnode *devnull;
static struct fnode *devzero;

static struct module mod_devnull = {
};


void devnull_init(struct fnode *dev)
{
    strcpy(mod_devnull.name,"devnull");
    mod_devnull.family = FAMILY_FILE;
    mod_devnull.ops.open = devnull_open;
    mod_devnull.ops.read = devnull_read; 
    mod_devnull.ops.poll = devnull_poll;
    mod_devnull.ops.write = devnull_write;

    devnull = fno_create(&mod_devnull, "null", dev);
    devzero = fno_create(&mod_devnull, "zero", dev);
    register_module(&mod_devnull);
}



