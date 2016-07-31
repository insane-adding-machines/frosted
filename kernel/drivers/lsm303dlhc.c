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
#include "dma.h"
#include "i2c.h"

#define LSM303_I2C_INS_ADDR     0x30
#define LSM303_I2C_COMPASS_ADDR 0x3C


enum lsm303_state {
    LSM303_STATE_IDLE = 0, 
    LSM303_STATE_BUSY,
    LSM303_STATE_TX_RDY,
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
} INS;

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


/* I2C operation callbacks, executed in IRQ context, and with I2C mutex held. */
static void isr_tx(struct i2c_slave * arg)
{
    struct dev_lsm303dlhc *lsm303dlhc = (struct dev_lsm303dlhc *) arg;
    lsm303dlhc->state = LSM303_STATE_TX_RDY;
    if (lsm303dlhc->dev->pid > 0)
        task_resume(lsm303dlhc->dev->pid);
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


static int devlsm303dlhc_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    struct dev_lsm303dlhc *lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);
    struct lsm303dlhc_ctrl_reg * ctrl = (struct lsm303dlhc_ctrl_reg *) arg;
    static uint8_t buffer;

    if (!lsm303dlhc)
        return -EINVAL;
    switch (lsm303dlhc->state) {
        case LSM303_STATE_IDLE:
            if(cmd == IOCTL_LSM303DLHC_READ_CTRL_REG) {
                lsm303dlhc->state = LSM303_STATE_BUSY;
                i2c_init_read(&lsm303dlhc->i2c, ctrl->reg, &buffer, 1);
            } else {
                buffer = ctrl->data;
                lsm303dlhc->state = LSM303_STATE_BUSY;
                i2c_init_write(&lsm303dlhc->i2c, ctrl->reg, &buffer, 1);
            }
            lsm303dlhc->dev->pid = scheduler_get_cur_pid();
            task_suspend();
            return SYS_CALL_AGAIN;

        case LSM303_STATE_RX_RDY:
            ctrl->data = buffer;
            lsm303dlhc->state = LSM303_STATE_IDLE;
            return 0;

        case LSM303_STATE_BUSY: 
        case LSM303_STATE_TX_RDY: 
            task_suspend();
            return SYS_CALL_AGAIN;
    }
}

static int devlsm303dlhc_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct dev_lsm303dlhc *lsm303dlhc = FNO_MOD_PRIV(fno, &mod_devlsm303dlhc);

    if (!lsm303dlhc)
        return -EINVAL;
    switch (lsm303dlhc->state) {
        case LSM303_STATE_IDLE:
            /* TODO: Implement Acceleration read mechanism (currently in userspace) */
            return 0;
            lsm303dlhc->dev->pid = scheduler_get_cur_pid();
            task_suspend();
            return SYS_CALL_AGAIN;

        case LSM303_STATE_RX_RDY:
            lsm303dlhc->state = LSM303_STATE_IDLE;
            return 0;

        case LSM303_STATE_BUSY: 
        case LSM303_STATE_TX_RDY: 
            task_suspend();
            return SYS_CALL_AGAIN;
    }
}

static int devlsm303dlhc_close(struct fnode *fno)
{
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
