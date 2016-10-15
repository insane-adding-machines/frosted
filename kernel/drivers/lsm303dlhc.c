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
#include "exti.h"
#include "dma.h"
#include "i2c.h"

#define LSM303_I2C_INS_ADDR     0x30
#define LSM303_I2C_COMPASS_ADDR 0x3C


enum lsm303_state {
    LSM303_STATE_DISABLED = 0,
    LSM303_STATE_IDLE,
    LSM303_STATE_BUSY,
    LSM303_STATE_RX_RDY
};

struct lsm303dlhc_ctrl_reg
{
    uint8_t reg;
    uint8_t data;
};

struct dev_lsm303dlhc {
    struct i2c_slave i2c; /* As first argument, so isr callbacks will use this as arg */
    struct device * dev;
    enum lsm303_state state;
    union dev_lsm303dlhc_data {
        uint8_t data[6];
        struct __attribute__((packed)) ins_data_xyz {
            int16_t x;
            int16_t y;
            int16_t z;
        } xyz;
    } ins_data;
} INS;

static int devlsm303dlhc_read(struct fnode *fno, void *buf, unsigned int len);
static int devlsm303dlhc_close(struct fnode *fno);

static struct module mod_devlsm303dlhc = {
    .family = FAMILY_FILE,
    .name = "lsm303dlhc",
    .ops.open = device_open,
    .ops.read = devlsm303dlhc_read,
    .ops.close = devlsm303dlhc_close,
};


/* I2C operation callbacks, executed in IRQ context, and with I2C mutex held. */
static void isr_tx(struct i2c_slave * arg)
{
    struct dev_lsm303dlhc *lsm303dlhc = (struct dev_lsm303dlhc *) arg;
    switch (lsm303dlhc->state) {
        case LSM303_STATE_DISABLED:
            lsm303dlhc->state = LSM303_STATE_IDLE;
            task_resume(lsm303dlhc->dev->pid);
            break;
        case LSM303_STATE_IDLE:
            lsm303dlhc->state = LSM303_STATE_DISABLED;
            break;
    }
}

static void isr_rx(struct i2c_slave * arg)
{
    struct dev_lsm303dlhc *lsm303dlhc = (struct dev_lsm303dlhc *) arg;
    lsm303dlhc->state = LSM303_STATE_RX_RDY;
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


#define CTRL_REG1_A (0x20)
#define OUT_X_L_A   (0x28)

static int devlsm303dlhc_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_lsm303dlhc *lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);
    uint8_t val;

    if (!lsm303dlhc)
        return -EINVAL;

    if (len == 0)
        return -EINVAL;

    switch (lsm303dlhc->state) {
        case LSM303_STATE_DISABLED:
            lsm303dlhc->dev->pid = scheduler_get_cur_pid();
            val = 0x27; /* PM=001, DR=00, XYZ=111 */
            i2c_init_write(&lsm303dlhc->i2c, CTRL_REG1_A, &val, 1);
            task_suspend();
            return SYS_CALL_AGAIN;
        case LSM303_STATE_IDLE:
            lsm303dlhc->dev->pid = scheduler_get_cur_pid();
            i2c_init_write(&lsm303dlhc->i2c, OUT_X_L_A, lsm303dlhc->ins_data.data, 6);
            task_suspend();
            return SYS_CALL_AGAIN;

        case LSM303_STATE_RX_RDY:
            if (len > 6)
                len = 6;
            memcpy(buf, lsm303dlhc->ins_data.data, len);
            lsm303dlhc->state = LSM303_STATE_IDLE;
            val = 0x07; /* PM=000, DR=00, XYZ=111 */
            i2c_init_write(&lsm303dlhc->i2c, CTRL_REG1_A, &val, 1);
            return len;
    }
}

static int devlsm303dlhc_close(struct fnode *fno)
{
    struct dev_lsm303dlhc *lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);
    uint8_t val = 0x07; /* PM=000, DR=00, XYZ=111 */
    lsm303dlhc->state = LSM303_STATE_IDLE;
    i2c_init_write(&lsm303dlhc->i2c, CTRL_REG1_A, &val, 1);
    return 0;
}

int lsm303dlhc_init(int bus)
{
    struct fnode *devfs;
    memset(&INS, 0, sizeof(struct dev_lsm303dlhc));

    devfs = fno_search("/dev");
    if (!devfs)
        return -EFAULT;

    INS.dev = device_fno_init(&mod_devlsm303dlhc, "ins", devfs, FL_RDONLY, &INS);
    if (!INS.dev)
        return -EFAULT;

    /* Populate i2c_slave struct */
    INS.i2c.bus = bus;
    INS.i2c.address = LSM303_I2C_INS_ADDR;
    INS.i2c.isr_tx = isr_tx;
    INS.i2c.isr_rx = isr_rx;

    register_module(&mod_devlsm303dlhc);
    return 0;
}
