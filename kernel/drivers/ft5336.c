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
 *      Authors: Daniele Lacamera <root@danielinux.net>
 *
 */
 
#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "cirbuf.h"
#include "dma.h"
#include "cirbuf.h"
#include "i2c.h"

#define FT5336_I2C_ADDR (0x70)
#define MAX_W   (480)
#define MAX_H   (272)

#define FT5336_MODE         (0x00)
#define FT5336_GESTURE_ID   (0x01)
#define FT5336_TOUCHDATA    (0x02)
#   define TD_PRESS         (0x00)
#   define TD_RELEASE       (0x01)
#   define TD_CONTACT       (0x02)
#define FT5336_P1_XH        (0x03)
#define FT5336_P1_YH        (0x05)

#define FT5336_G_MODE       (0xA4)
#define FT5336_G_MODE_INTERRUPT (0x01)

#define FT5336_CHIP_ID      (0xA8)
#define FT5336_CHIP_ID_VAL  (0x51)

/* Single device supported in this driver. */
#define ST_OFF 0
#define ST_RDY 1
#define ST_ON  2


struct dev_ts {
    struct i2c_slave i2c;
    struct device *dev;
    uint8_t i2c_data;
    int state;
} Ts;



/* Module description */
int ft5336_read(struct fnode *fno, void *buf, unsigned int len)
{
    return 0;
}

static struct module mod_devts = {
    .family = FAMILY_FILE,
    .name = "ts",
    .ops.open = device_open,
    .ops.read = ft5336_read
};


static void ft5336_task(void *arg)
{
    (void)arg;
    uint8_t val;

    if (i2c_kthread_read(&Ts.i2c, FT5336_CHIP_ID, &val, 1) > 0) {
        if (val != FT5336_CHIP_ID_VAL)
            return 0; /* kthread terminated. */
    }
    
    val = FT5336_G_MODE_INTERRUPT;
    i2c_kthread_write(&Ts.i2c, FT5336_G_MODE, &val, 1);
}

int ft5336_init(uint32_t bus)
{
    int i;
    struct fnode *devdir = fno_search("/dev");
    if (!devdir)
        return -ENOENT;
    memset(&Ts, 0, sizeof(struct dev_ts));
    Ts.dev = device_fno_init(&mod_devts, "ts", devdir, 0, &Ts);

    /* Populate i2c_slave struct */
    Ts.i2c.bus = bus;
    Ts.i2c.address = FT5336_I2C_ADDR;
    kthread_create(ft5336_task, NULL);
    return 0;
}
