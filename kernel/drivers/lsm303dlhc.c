/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
 *
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors:
 *
 */

#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "ioctl.h"
#include "lsm303dlhc.h"
#include "gpio.h"
#include "stm32_exti.h"
#include "stm32f4_dma.h"
#include "stm32f4_i2c.h"

typedef enum
{
    LSM303DLHC_IDLE,
    LSM303DLHC_READ,
    LSM303DLHC_WRITE,
}LSM303DLHC_MODE;

struct dev_lsm303dlhc {
    struct device * dev;
    struct fnode *i2c_fnode;
    struct fnode *int_1_fnode;
    struct fnode *int_2_fnode;
    struct fnode *drdy_fnode;
    uint8_t address;
    LSM303DLHC_MODE mode;
};

struct lsm303dlhc_ctrl_reg
{
    uint8_t reg;
    uint8_t data;
};

#define MAX_LSM303DLHC 2

static struct dev_lsm303dlhc DEV_LSM303DLHC[MAX_LSM303DLHC];

static int devlsm303dlhc_read(struct fnode *fno, void *buf, unsigned int len);
static int devlsm303dlhc_ioctl(struct fnode * fno, const uint32_t cmd, void *arg);
static int devlsm303dlhc_close(struct fnode *fno);

static struct module mod_devlsm303dlhc = {
    .family = FAMILY_FILE,
    .name = "lsm303dlhc",
    .ops.open = device_open,
    .ops.read = devlsm303dlhc_read,
    .ops.ioctl = devlsm303dlhc_ioctl,
    .ops.close = devlsm303dlhc_close,
};

static void completion(void * arg)
{
    const struct dev_lsm303dlhc *lsm303dlhc = (struct dev_lsm303dlhc *) arg;

    if (lsm303dlhc->dev->pid > 0)
        task_resume(lsm303dlhc->dev->pid);
}


static void int1_callback(void *arg)
{
    (void*)arg;
}

static void int2_callback(void *arg)
{
    (void*)arg;
}

static void drdy_callback(void *arg)
{
    (void*)arg;
}


static int devlsm303dlhc_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    struct dev_lsm303dlhc *lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);
    struct lsm303dlhc_ctrl_reg * ctrl = (struct lsm303dlhc_ctrl_reg *) arg;
    static uint8_t buffer;

    if (!lsm303dlhc)
        return -1;

    if(lsm303dlhc->mode == LSM303DLHC_IDLE)
    {
        lsm303dlhc->dev->pid = scheduler_get_cur_pid();
        task_suspend();

        if(cmd == IOCTL_LSM303DLHC_READ_CTRL_REG)
        {
            lsm303dlhc->mode = LSM303DLHC_READ;
            i2c_read(lsm303dlhc->i2c_fnode, completion, lsm303dlhc, lsm303dlhc->address, ctrl->reg, &buffer, 1);
        }
        else
        {
            buffer = ctrl->data;
            lsm303dlhc->mode = LSM303DLHC_WRITE;
            i2c_write(lsm303dlhc->i2c_fnode, completion, lsm303dlhc, lsm303dlhc->address, ctrl->reg, &buffer, 1);
        }

        return SYS_CALL_AGAIN;
    }

    if(lsm303dlhc->mode == LSM303DLHC_READ)
    {
        ctrl->data = buffer;
    }
    lsm303dlhc->mode = LSM303DLHC_IDLE;

    return 0;
}

static int devlsm303dlhc_read(struct fnode *fno, void *buf, unsigned int len)
{
    int i;
    char *ch = (char *)buf;
    const struct dev_lsm303dlhc *lsm303dlhc;

    if (len <= 0)
        return len;

    lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);
    if (!lsm303dlhc)
        return -1;


    i2c_read(lsm303dlhc->i2c_fnode, completion, (void*)lsm303dlhc, lsm303dlhc->address, 0, buf, len);

    return SYS_CALL_AGAIN;


    return len;
}
static int devlsm303dlhc_close(struct fnode *fno)
{
    return 0;
}


static void lsm303dlhc_fno_init(struct fnode *dev, const struct lsm303dlhc_addr *addr)
{
    struct dev_lsm303dlhc *l = &DEV_LSM303DLHC[0];
    l->dev = device_fno_init(&mod_devlsm303dlhc, addr->name, dev, FL_RDWR, l);
    l->i2c_fnode = fno_search(addr->i2c_name);
    l->address = addr->address;
    l->mode = LSM303DLHC_IDLE;
}

void lsm303dlhc_init(struct fnode *dev, const struct lsm303dlhc_addr addr)
{
    int i;
    exti_register(addr.pio1_base, addr.pio1_pin, EXTI_TRIGGER_RISING, int1_callback, NULL);
    exti_register(addr.pio2_base, addr.pio2_pin, EXTI_TRIGGER_RISING, int2_callback, NULL);
    exti_register(addr.drdy_base, addr.drdy_pin, EXTI_TRIGGER_RISING, drdy_callback, NULL); 

    lsm303dlhc_fno_init(dev, &addr);
    register_module(&mod_devlsm303dlhc);
}
