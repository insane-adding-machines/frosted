#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "l3gd20.h"

struct dev_l3gd20 {
    struct device * dev;
    uint32_t irq;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
};

#define MAX_L3GD20S 1 

static struct dev_l3gd20 DEV_L3GD20S[MAX_L3GD20S];

static int devl3gd20_read(int fd, void *buf, unsigned int len);
static int devl3gd20_write(int fd, const void *buf, unsigned int len);


static struct module mod_devl3gd20 = {
    .family = FAMILY_FILE,
    .name = "l3gd20",
    .ops.open = device_open,
    .ops.read = devl3gd20_read, 
    .ops.write = devl3gd20_write,
};

static int devl3gd20_write(int fd, const void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    const struct dev_l3gd20 *l3gd20;

    l3gd20 = device_check_fd(fd, &mod_devl3gd20);
    if (!l3gd20)
        return -1;
    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;



    return len;
}


static int devl3gd20_read(int fd, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    const struct dev_spi *spi;
    const struct dev_l3gd20 *l3gd20;

    if (len <= 0)
        return len;
    if (fd < 0)
        return -1;

    l3gd20 = device_check_fd(fd, &mod_devl3gd20);
    if (!l3gd20)
        return -1;





    return out;
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
        l3gd20_fno_init(dev, i, &l3gd20_addrs[i]);
//        l3gd20_enable(l3gd20_addrs[i].base);
        
        f = device_open("/dev/spi1", 0);


    }
    register_module(&mod_devl3gd20);
}

