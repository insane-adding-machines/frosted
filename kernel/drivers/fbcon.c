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
 *      Authors: Daniele Lacamera
 *
 */
 
#include "frosted.h"
#include "device.h"
#include <stdint.h>
#include "poll.h"
#include "framebuffer.h"

#define FBCON_L 60
#define FBCON_H 34

#define SCR_ROWS 480
#define SCR_COLS 272

struct dev_fbcon {
    uint16_t sid;
    uint16_t size_x;
    uint16_t size_y;
    uint16_t cursor;
    unsigned char *buffer;
    unsigned char *screen;
};

static int devfbcon_write(struct fnode *fno, const void *buf, unsigned int len);
static int devfbcon_read(struct fnode *fno, void *buf, unsigned int len);
static int devfbcon_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static void devfbcon_tty_attach(struct fnode *fno, int pid);

extern const unsigned char fb_font[256][8];

static struct module mod_devfbcon = {
    .family = FAMILY_FILE,
    .name = "fbcon",
    .ops.open = device_open,
    .ops.read = devfbcon_read,
    .ops.poll = devfbcon_poll,
    .ops.write = devfbcon_write,
    .ops.tty_attach = devfbcon_tty_attach,
};


static void devfbcon_tty_attach(struct fnode *fno, int pid)
{
    struct dev_fbcon *fbcon = (struct dev_fbcon *)FNO_MOD_PRIV(fno, &mod_devfbcon);
    if (fbcon->sid != pid) {
        //kprintf("/dev/%s active job pid: %d\r\n", fno->fname, pid);
        fbcon->sid = pid;
    }
}

static void render_row(struct dev_fbcon *fbcon, int row)
{
    int i, j, l;
    unsigned char fc;
    unsigned char fcl;
    int screen_off;

    for (l = 0; l < 8; l++) { /* For each screen line... */
        for (i = 0; i < FBCON_L; i++) { /* Each char */
            fc = fbcon->buffer[row * FBCON_L + i];
            fcl = fb_font[fc][l];
            for (j = 0; j < 8; j++) {
                screen_off = ((row * 8 + l) * SCR_COLS) + (i * 8) + j;
                fbcon->screen[screen_off] = ((fcl & (1 << j)) >> j)?0xFF:0x00;
            }
        }
    }
}

static void render_screen(struct dev_fbcon *fbcon)
{
    int i;
    for (i = 0; i < FBCON_H; i++)
        render_row(fbcon, i);
}

static int devfbcon_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i;
    struct dev_fbcon *fbcon = (struct dev_fbcon *)FNO_MOD_PRIV(fno, &mod_devfbcon);
    const uint8_t *cbuf = buf;
    for (i = 0; i < len; i++) {
        fbcon->buffer[fbcon->cursor++] = cbuf[i];
        if ((fbcon->cursor) >= (FBCON_L * FBCON_H)) {
            /* TODO: scroll. Wrap around for now. */
            fbcon->cursor = 0;
        }
    }
    render_screen(fbcon);
    return len;
}


static int devfbcon_read(struct fnode *fno, void *buf, unsigned int len)
{
    return -EINVAL;
}


static int devfbcon_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    return 1;
}

int fbcon_init(void)
{
    struct fnode *devfs = fno_search("/dev");
    struct dev_fbcon *fbcon = kalloc(sizeof(struct dev_fbcon));
    struct fnode *fno_fbcon;
    if (!fbcon)
        return -1;

    fbcon->buffer = kalloc(sizeof(struct dev_fbcon));
    if (!fbcon->buffer) {
        kfree(fbcon);
        return -1;
    }

    if (devfs == NULL) {
        kfree(fbcon->buffer);
        kfree(fbcon);
        return -1;
    }

    memset(fbcon, 0, sizeof(struct dev_fbcon));
    memset(fbcon->buffer, 0, (FBCON_L * FBCON_H));
    register_module(&mod_devfbcon);
    fno_fbcon = fno_create(&mod_devfbcon, "fbcon", devfs);
    fno_fbcon->priv = fbcon;
    fbcon->size_x = FBCON_L;
    fbcon->size_y = FBCON_H;
    fbcon->screen = framebuffer_get();
    return 0;
}
