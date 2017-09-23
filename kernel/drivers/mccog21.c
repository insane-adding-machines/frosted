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

#define MCCOG21_I2C_ADDR (0x7c)

#define MCC0G21_CLEAR_DISPLAY (0x01)
#define MCC0G21_RETURN_HOME   (0x01)
#define MCC0G21_ENTRY_MODE    (0x04)
#   define MCC0G21_ENTRY_MODE_INCREMENT   (0x02)
#   define MCC0G21_ENTRY_MODE_DECREMENT   (0x00)
#   define MCC0G21_ENTRY_SHIFT_ON         (0x01)
#   define MCC0G21_ENTRY_SHIFT_OFF        (0x00)
#define MCC0G21_ONOFF         (0x08)
#   define MCC0G21_DISPLAY_ON       (1 << 2)
#   define MCC0G21_CURSOR_ON        (1 << 1)
#   define MCC0G21_CURSOR_BLINK     (1 << 0)

#define MCC0G21_SHIFT         (0x10)

#define MCC0G21_OSC_FREQ (0x10)
#   define MCC0G21_OSC_FREQ_192HZ (0x04)
#define MCC0G21_FSET          (0x20)
#       define MCC0G21_FSET_8BIT        (1 << 4)
#       define MCC0G21_FSET_TWOLINES    (1 << 3)
#       define MCC0G21_FSET_DOUBLEH     (1 << 2)

#       define MCC0G21_FSET_EXTEND      (1 << 0)

#define MCC0G21_CGRAM_SELECT  (0x40)
#define MCC0G21_DDRAM_SELECT  (0x80)
#define MCC0G21_CONTRAST_HI   (0x50)
#define MCC0G21_FOLLOWER      (0x60)
#       define MCC0G21_FOLLOWER_5V      (0x0C)
#define MCC0G21_CONTRAST_LO   (0x70)


#define MCC0G21_COMMAND_PREFIX (0x00)
#define MCC0G21_DATA_PREFIX    (0x40)

struct dev_disp {
    struct i2c_slave i2c;
    struct device *dev;
    char buf[80];
    uint8_t functions;
    volatile int update;
} Disp;

static void mccog21_clear(void)
{
    uint8_t cmd = MCC0G21_CLEAR_DISPLAY;
    int r;
    kthread_sleep_ms(400);
    do {
        r = i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    } while (r != 1);
    kthread_sleep_ms(800);
}

/* Module description */
int mccog21_read(struct fnode *fno, void *buf, unsigned int len)
{
    if (len > 80)
        len = 80;
    if (len > 0)
        memcpy(buf, Disp.buf, len);
    return len;
}

int mccog21_write(struct fnode *fno, const void *buf, unsigned int len)
{
    memset(Disp.buf, 0, 80);
    if (len > 80)
        len = 80;
    if (len > 0)
        memcpy(Disp.buf, buf, len);

    Disp.update = 1;
    return len;
}

static struct module mod_devdisp = {
    .family = FAMILY_FILE,
    .name = "lcd",
    .ops.open = device_open,
    .ops.read = mccog21_read,
    .ops.write= mccog21_write
};


static uint8_t cmd;

static void mccog21_task(void *arg)
{
    (void)arg;
    int i;
    int r;

    kthread_sleep_ms(80); /* Wait 40ms until VDD stabilizes */
    /* Mode = 8 bit, two lines */
    Disp.functions =  MCC0G21_FSET_8BIT | MCC0G21_FSET_TWOLINES;
    cmd = MCC0G21_FSET | Disp.functions;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

    /* Mode = 8 bit, two lines */
    Disp.functions =  MCC0G21_FSET_8BIT | MCC0G21_FSET_TWOLINES;
    cmd = MCC0G21_FSET | Disp.functions;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

    /* Switch to extended mode */
    cmd = MCC0G21_FSET | Disp.functions | MCC0G21_FSET_EXTEND;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

    /* Oscillator */
    cmd = MCC0G21_OSC_FREQ | MCC0G21_OSC_FREQ_192HZ;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    kthread_sleep_ms(200);

    /* Follower circuit off */
    cmd = MCC0G21_FOLLOWER | MCC0G21_FOLLOWER_5V;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    kthread_sleep_ms(200);

    /* Switch to normal mode */
    cmd = MCC0G21_FSET | Disp.functions;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

    /* Power on */
    cmd = MCC0G21_ONOFF | MCC0G21_DISPLAY_ON;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);


    mccog21_clear();


    cmd = MCC0G21_RETURN_HOME;;
    do {
        r = i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    } while (r != 1);

    kthread_sleep_ms(800);

    cmd = MCC0G21_ENTRY_MODE | MCC0G21_ENTRY_MODE_INCREMENT;
    do {
        r = i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    } while (r != 1);

    /* Switch to extended mode */
    cmd = MCC0G21_FSET | Disp.functions | MCC0G21_FSET_EXTEND;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

    /* Set contrast */
    cmd = MCC0G21_CONTRAST_HI | 0x02;
    do {
        r = i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
    } while (r < 1);
    /* Switch to normal mode */
    cmd = MCC0G21_FSET | Disp.functions;
    i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);


    while(1<2) {
        if (Disp.update) {
            cmd = MCC0G21_RETURN_HOME;;
            r = i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);
            kthread_sleep_ms(80);
            cmd = MCC0G21_DDRAM_SELECT;
            i2c_kthread_write(&Disp.i2c, MCC0G21_COMMAND_PREFIX, &cmd, 1);

            for (i = 0; i < 80; i++) {
                if (Disp.buf[i] == 0)
                    break;
                i2c_kthread_write(&Disp.i2c, MCC0G21_DATA_PREFIX, &Disp.buf[i], 1);
            }
            Disp.update = 0;
        }
        kthread_yield();
    }
}

int mccog21_init(uint32_t bus)
{
    int i;
    struct fnode *devdir = fno_search("/dev");
    if (!devdir)
        return -ENOENT;
    memset(&Disp, 0, sizeof(struct dev_disp));
    Disp.dev = device_fno_init(&mod_devdisp, "lcd", devdir, 0, &Disp);

    /* Populate i2c_slave struct */
    Disp.i2c.bus = bus;
    Disp.i2c.address = MCCOG21_I2C_ADDR;
    strcpy(Disp.buf, "Frosted");
    Disp.update = 1;
    kthread_create(mccog21_task, NULL);
    return 0;
}
