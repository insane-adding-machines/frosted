#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "l3gd20.h"
#include "l3gd20_ioctl.h"
#include "gpio.h"
#include "stm32f4_exti.h"

typedef enum
{
    L3GD20_IDLE,
    L3GD20_READ,
    L3GD20_WRITE,
    L3GD20_PENDING,
    L3GD20_READING
}L3GD20_MODE;

struct dev_l3gd20 {
    struct device * dev;
    uint32_t irq;
    struct fnode *spi_fnode;
    struct fnode *cs_fnode;
    struct fnode *int_1_fnode;
    struct fnode *int_2_fnode;
    L3GD20_MODE mode;
};

#define MAX_L3GD20S 1 

static struct dev_l3gd20 DEV_L3GD20S[MAX_L3GD20S];

static int devl3gd20_read(struct fnode *fno, void *buf, unsigned int len);
static int devl3gd20_write(struct fnode *fno, const void *buf, unsigned int len);
static int devl3gd20_ioctl(struct fnode * fno, const uint32_t cmd, void *arg);
static int devl3gd20_close(struct fnode *fno);

static struct module mod_devl3gd20 = {
    .family = FAMILY_FILE,
    .name = "l3gd20",
    .ops.open = device_open,
    .ops.read = devl3gd20_read, 
    .ops.write = devl3gd20_write,
    .ops.ioctl = devl3gd20_ioctl,
    .ops.close = devl3gd20_close,
};

static void spi_completion(void * arg)
{
    const struct dev_l3gd20 *l3gd20 = (struct dev_l3gd20 *) arg;
    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "1", 1);
    
    if (l3gd20->dev->pid > 0) 
        task_resume(l3gd20->dev->pid);
}

static void int1_callback(void * arg)
{
    const struct dev_l3gd20 *l3gd20 = (struct dev_l3gd20 *) arg;

}


static void int2_callback(void * arg)
{
    const struct dev_l3gd20 *l3gd20 = (struct dev_l3gd20 *) arg;
    l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "1", 1);

    if (l3gd20->dev->pid > 0) 
        task_resume(l3gd20->dev->pid);
}

static uint8_t ioctl_ibuffer[2];
static uint8_t ioctl_obuffer[2];

static int devl3gd20_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    struct dev_l3gd20 *l3gd20 = FNO_MOD_PRIV(fno, &mod_devl3gd20);
    struct l3gd20_ctrl_reg * ctrl = (struct l3gd20_ctrl_reg *) arg;

    if (!l3gd20)
        return -1;

    if(l3gd20->mode == L3GD20_IDLE)
    {
        ioctl_obuffer[0] = ctrl->reg;

        if(cmd == IOCTL_L3GD20_READ_CTRL_REG)
        {
            ioctl_obuffer[0] |= 0x80;
            ioctl_obuffer[1] = 0;
            l3gd20->mode = L3GD20_READ;
        }
        else
        {
            ioctl_obuffer[1] = ctrl->data;
            l3gd20->mode = L3GD20_WRITE;
        }

        l3gd20->dev->pid = scheduler_get_cur_pid();
        task_suspend();

        l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "0", 1);
        devspi_xfer(l3gd20->spi_fnode, spi_completion, l3gd20,  ioctl_obuffer, ioctl_ibuffer, 2);

        return SYS_CALL_AGAIN;
    }

    if(l3gd20->mode == L3GD20_READ)
    {
        ctrl->data = ioctl_ibuffer[1];
    }
    l3gd20->mode = L3GD20_IDLE;
    
    return 0;
}

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
    const struct dev_spi *spi;
    struct dev_l3gd20 *l3gd20 = FNO_MOD_PRIV(fno, &mod_devl3gd20);

    static uint8_t rd_ibuffer[7];
    static uint8_t rd_obuffer[7];
    
    if (len <= 0)
        return len;
    
    if (!l3gd20)
        return -1;

    /* First read is a fake just to get the DRDY IRQ going - what a bloody awful gyro*/
    if(l3gd20->mode == L3GD20_IDLE)
    {
        exti_enable(l3gd20->int_2_fnode, 1);

        l3gd20->mode = L3GD20_PENDING;
        rd_obuffer[0] = 0xE8;

        l3gd20->dev->pid = scheduler_get_cur_pid();
        task_suspend();

        l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "0", 1);
        devspi_xfer(l3gd20->spi_fnode, spi_completion, l3gd20,  rd_obuffer, rd_ibuffer, 7);
        return SYS_CALL_AGAIN;
    }
    else if(l3gd20->mode == L3GD20_PENDING)
    {
        l3gd20->mode = L3GD20_READING;
        rd_obuffer[0] = 0xE8;
        
        l3gd20->dev->pid = scheduler_get_cur_pid();
        task_suspend();

        l3gd20->cs_fnode->owner->ops.write(l3gd20->cs_fnode, "0", 1);
        devspi_xfer(l3gd20->spi_fnode, spi_completion, l3gd20,  rd_obuffer, rd_ibuffer, 7);
        return SYS_CALL_AGAIN;
    }
    else if(l3gd20->mode == L3GD20_READING) 
    {
        if(len > 6)
            len = 6;
        exti_enable(l3gd20->int_2_fnode, 0);
        memcpy(buf, &rd_ibuffer[1], len);
        l3gd20->mode = L3GD20_PENDING;
    }
    return len;
}

static int devl3gd20_close(struct fnode *fno)
{
    struct dev_l3gd20 *l3gd20;
    l3gd20 = FNO_MOD_PRIV(fno, &mod_devl3gd20);
    if (!l3gd20)
        return -1;
    l3gd20->mode = L3GD20_IDLE;

    exti_enable(l3gd20->int_2_fnode , 0);
    return 0;
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
        l3gd20_fno_init(dev, i, &l3gd20_addrs[i]);
        DEV_L3GD20S[i].spi_fnode = device_find(dev, l3gd20_addrs[i].spi_name);
        DEV_L3GD20S[i].cs_fnode = device_find(dev, l3gd20_addrs[i].spi_cs_name);
        DEV_L3GD20S[i].int_1_fnode = device_find(dev, l3gd20_addrs[i].int_1_name);
        DEV_L3GD20S[i].int_2_fnode = device_find(dev, l3gd20_addrs[i].int_2_name);

        if(DEV_L3GD20S[i].int_1_fnode)exti_register_callback(DEV_L3GD20S[i].int_1_fnode, int1_callback, &DEV_L3GD20S[i]);
        if(DEV_L3GD20S[i].int_2_fnode)exti_register_callback(DEV_L3GD20S[i].int_2_fnode, int2_callback, &DEV_L3GD20S[i]);

        DEV_L3GD20S[i].cs_fnode->owner->ops.write(DEV_L3GD20S[i].cs_fnode, "1", 1);
        DEV_L3GD20S[i].mode = L3GD20_IDLE;
    }
    register_module(&mod_devl3gd20);
}

