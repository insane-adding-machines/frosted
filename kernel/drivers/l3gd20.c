#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "l3gd20.h"

struct dev_l3gd20 {
    struct device * dev;
    uint32_t irq;
    struct fnode *spi_fnode;
    struct fnode *cs_fnode;
    struct fnode *int_1_fnode;
    struct fnode *int_2_fnode;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
};

#define MAX_L3GD20S 1 

static struct dev_l3gd20 DEV_L3GD20S[MAX_L3GD20S];

static int devl3gd20_read(struct fnode *fno, void *buf, unsigned int len);
static int devl3gd20_write(struct fnode *fno, const void *buf, unsigned int len);


static struct module mod_devl3gd20 = {
    .family = FAMILY_FILE,
    .name = "l3gd20",
    .ops.open = device_open,
    .ops.read = devl3gd20_read, 
    .ops.write = devl3gd20_write,
};

static int devl3gd20_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    const struct dev_l3gd20 *l3gd20;

    if (len <= 0)
        return len;

    l3gd20 = FNO_MOD_PRIV(fno, &mod_devl3gd20);
    if (!l3gd20)
        return -1;



    return len;
}

static int devl3gd20_read(struct fnode *fno, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    const struct dev_spi *spi;
    const struct dev_l3gd20 *l3gd20;

    uint8_t p[2] = {0x20, 0x0F};
    uint8_t o[2] = {0x8F, 0x00};
    uint8_t i[2];

    if (len <= 0)
        return len;
    
    l3gd20 = FNO_MOD_PRIV(fno, &mod_devl3gd20);
    if (!l3gd20)
        return -1;

    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "0", 1);
    l3gd20->spi_fnode->owner->ops.write(l3gd20->spi_fnode, p, 2);
    l3gd20->spi_fnode->owner->ops.read(l3gd20->spi_fnode, i, 2);
    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "1", 1);

    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "0", 1);
    l3gd20->spi_fnode->owner->ops.write(l3gd20->spi_fnode, o, 2);
    l3gd20->spi_fnode->owner->ops.read(l3gd20->spi_fnode, i, 2);
    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "1", 1);

    return out;
}

struct fnode * device_find(const struct fnode *dev, const char * name)
{
    char path[256];
    memset(path, 0, 256);
    path[0] = '/';
    strcat(&path[1], dev->fname);
    path[4] =  '/';
    strcat(&path[5], name);
    return fno_search(path);
}

static void l3gd20_fno_init(struct fnode *dev, uint32_t n, const struct l3gd20_addr * addr)
{
    struct dev_l3gd20 *l = &DEV_L3GD20S[n];
    l->dev = device_fno_init(&mod_devl3gd20, addr->name, dev, FL_RDWR, l);
}


void l3gd20_init(struct fnode * dev, const struct l3gd20_addr l3gd20_addrs[], int num_l3gd20s)
{
    int i, f;
    for (i = 0; i < num_l3gd20s; i++) 
    {
        DEV_L3GD20S[i].spi_fnode = device_find(dev, l3gd20_addrs[i].spi_name);
        DEV_L3GD20S[i].cs_fnode = device_find(dev, l3gd20_addrs[i].spi_cs_name);
        DEV_L3GD20S[i].int_1_fnode = device_find(dev, l3gd20_addrs[i].int_1_name);
        DEV_L3GD20S[i].int_2_fnode = device_find(dev, l3gd20_addrs[i].int_2_name);
        l3gd20_fno_init(dev, i, &l3gd20_addrs[i]);
    }
    register_module(&mod_devl3gd20);
}

