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

#define XADOW_LED_I2C_ADDR (0x21 << 1)

#define DISP_CHAR_5X7                0x80
#define DISP_STRING                  0x81
#define SET_DISP_ORIENTATION         0x82
#define POWER_DOWN                   0x83
#define DISP_PIC                     0x84
#define DISP_DATA                    0x85

struct dev_matrix {
    struct i2c_slave i2c;
    struct device *dev;
    uint8_t i2c_data;
    uint8_t buf[20];
    int update;
} Matrix;

static struct task *kt = NULL;



/* Module description */
static int xadow_led_read(struct fnode *fno, void *buf, unsigned int len)
{
    return 0;
}


static int xadow_led_write(struct fnode *fno, const void *buf, unsigned int len)
{
    memset(Matrix.buf, 0, 20);
    if (len > 17)
        len = 17;
    Matrix.buf[0] = len;
    if (len > 0) {
        memcpy(Matrix.buf + 1, buf, len);
    }
    Matrix.buf[len + 1] = 0;
    Matrix.buf[len + 2] = 120;

    Matrix.update = len + 3;
    return len;
}

static void xadow_led_task(void *arg)
{
    (void)arg;
    int r;
    Matrix.update = 0;
    while(1<2) {
        if (Matrix.update) {
            r = i2c_kthread_write(&Matrix.i2c, DISP_STRING, Matrix.buf, Matrix.update);
            Matrix.update = 0;
        }
        kthread_yield();
    }
}

static int xadow_led_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    kt = kthread_create(xadow_led_task, NULL);
    return task_filedesc_add(f);
}

static int xadow_led_close(struct fnode *f)
{
    if (kt) {
        kthread_cancel(kt);
        kt = NULL;
    }
    return 0;
}

static struct module mod_devmatrix = {
    .family = FAMILY_FILE,
    .name = "matrix",
    .ops.open = xadow_led_open,
    .ops.read = xadow_led_read,
    .ops.write = xadow_led_write,
    .ops.close = xadow_led_close
};


int xadow_led_init(uint32_t bus)
{
    int i;
    struct fnode *devdir = fno_search("/dev");
    if (!devdir)
        return -ENOENT;
    memset(&Matrix, 0, sizeof(struct dev_matrix));
    Matrix.dev = device_fno_init(&mod_devmatrix, "matrix", devdir, 0, &Matrix);


    /* Populate i2c_slave struct */
    Matrix.i2c.bus = bus;
    Matrix.i2c.address = XADOW_LED_I2C_ADDR;
    //kthread_create(xadow_led_task, NULL);
    return 0;
}
