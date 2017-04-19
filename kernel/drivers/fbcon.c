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

#define SCR_ROWS 272
#define SCR_COLS 480
#define COLOR_DEFAULT 15 /* White */

struct dev_fbcon {
    struct device *dev;
    uint16_t sid;
    uint16_t size_x;
    uint16_t size_y;
    uint16_t cursor;
    uint8_t color;
    uint8_t escape;
    unsigned char *buffer;
    unsigned char *colormap;
    unsigned char *screen;
};

static int devfbcon_write(struct fnode *fno, const void *buf, unsigned int len);
static int devfbcon_read(struct fnode *fno, void *buf, unsigned int len);
static int devfbcon_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static void devfbcon_tty_attach(struct fnode *fno, int pid);
static int devfbcon_seek(struct fnode *fno, int off, int whence);

extern const unsigned char fb_font[256][8];
extern const uint32_t xterm_cmap[256];

static struct module mod_devfbcon = {
    .family = FAMILY_FILE,
    .name = "fbcon",
    .ops.open = device_open,
    .ops.read = devfbcon_read,
    .ops.poll = devfbcon_poll,
    .ops.write = devfbcon_write,
    .ops.seek = devfbcon_seek,
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
    const unsigned char *fcl;
    uint8_t fc_color = COLOR_DEFAULT;
    int screen_off;

    for (i = 0; i < FBCON_L; i++) {            /* Each char ... i = column */ 
        fc = fbcon->buffer[row * FBCON_L + i]; /* fc = char to render */
        fc_color = 
            fbcon->colormap[row * FBCON_L + i];/* fc_color: char color */
        fcl = fb_font[fc];                     /* fcl = font rendering, 8 bytes. */
        for (l = 0; l < 8; l++) {              /* screen lines 0..7 */
            for (j = 0; j < 8; j++) {          /* each pixel in screen line */
                screen_off = ((row * 8 + l) * SCR_COLS) + (i * 8) + j;
                fbcon->screen[screen_off] = ((fcl[l] & (1 << (7 - j))) >> (7 - j))?fc_color:0;
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

static void scroll(struct dev_fbcon *fbcon)
{
    unsigned char *dest = fbcon->buffer;
    unsigned char *src = fbcon->buffer + FBCON_L;
    unsigned char *c_dest = fbcon->colormap;
    unsigned char *c_src = fbcon->colormap + FBCON_L;
    int i;
    for (i = 0; i < FBCON_H; i++) {
        memcpy(dest, src, FBCON_L);
        dest += FBCON_L;
        src += FBCON_L;
        memcpy(c_dest, c_src, FBCON_L);
        c_dest += FBCON_L;
        c_src += FBCON_L;
    }
    fbcon->cursor = FBCON_L * (FBCON_H - 1);
    memset(fbcon->buffer + fbcon->cursor, 0, FBCON_L);
    memset(fbcon->colormap + fbcon->cursor, COLOR_DEFAULT, FBCON_L);
}

static int devfbcon_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i, j;
    struct dev_fbcon *fbcon = (struct dev_fbcon *)FNO_MOD_PRIV(fno, &mod_devfbcon);
    const uint8_t *cbuf = buf;

    for (i = 0; i < len; i++) {
        int p = 0, t = 0;
        if ((fbcon->cursor) >= (FBCON_L * FBCON_H)) {
            scroll(fbcon);
        }

        if (fbcon->escape == 0x1b) {
            if (cbuf[i] == '[')
                fbcon->escape = '[';
            else { 
                fbcon->color = cbuf[i];
                fbcon->escape = 0;
            }
            continue;
        }
        if (fbcon->escape == '[') {
            if (cbuf[i] == 'H') {
                fbcon->cursor = 0;
            }
            if (cbuf[i] == 'J') {
                for (j = fbcon->cursor; j < (FBCON_L * FBCON_H); j++) {
                    fbcon->buffer[j] = 0x20;
                }
            }
            fbcon->escape = 0;
            continue;
        }
        // TODO fbcon->color = cbuf[i];

        switch(cbuf[i]) {
            case '\r':
                fbcon->cursor -= (fbcon->cursor % FBCON_L);
                break;

            /* LF */
            case '\n': 
                fbcon->cursor += FBCON_L;
                break;

            /* FF */
            case 0x0c:
                memset(fbcon->buffer, 0, FBCON_L * FBCON_H);
                fbcon->cursor = 0;
                break;

            /* TAB */
            case '\t': 
                t = fbcon->cursor % 4;
                if (t == 0) 
                    t = 4;
                for (p = 0; p < t; p++)
                    fbcon->buffer[fbcon->cursor + p] = 0x20;
                fbcon->cursor += t;
                break;


            /* BS */
            case 0x08:
                if (fbcon->cursor > 0) {
                    fbcon->cursor--;
                    fbcon->buffer[fbcon->cursor] = 0x20;
                    fbcon->colormap[fbcon->cursor] = COLOR_DEFAULT;
                }
                break;


            /* DEL */
            case 0x7f:
                fbcon->buffer[fbcon->cursor] = 0x20;
                fbcon->colormap[fbcon->cursor] = COLOR_DEFAULT;
                break;

            /* ESC */
            case 0x1b:
                fbcon->escape = 0x1b;
                break;

            /* Printable char */
            default:
                fbcon->colormap[fbcon->cursor] = fbcon->color;
                fbcon->buffer[fbcon->cursor++] = cbuf[i];
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

static int devfbcon_seek(struct fnode *fno, int off, int whence)
{
    int new_off;
    struct dev_fbcon *fbcon = (struct dev_fbcon *)FNO_MOD_PRIV(fno, &mod_devfbcon);
    switch(whence) {
        case SEEK_CUR:
            new_off = fbcon->cursor + off;
            break;
        case SEEK_SET:
            new_off = off;
            break;
        case SEEK_END:
            new_off = FBCON_L * FBCON_H + off;
            break;
        default:
            return -EINVAL;
    }

    if (new_off < 0)
        new_off = 0;

    if (new_off > FBCON_L * FBCON_H) {
        new_off = FBCON_L * FBCON_H;
    }
    fbcon->cursor = new_off;
    return 0;
}


const char frosted_banner[] = "\r\n~~~ Welcome to Frosted! ~~~\r\n";
const char fbcon_banner[] = "\r\nConsole framebuffer enabled.\r\n";

static const unsigned char color[2] = { 0x1b, 13 };
static const unsigned char white[2] = { 0x1b, 15 };

int fbcon_init(void)
{
    struct fnode *devfs = fno_search("/dev");
    struct dev_fbcon *fbcon = kalloc(sizeof(struct dev_fbcon));
    struct fnode *fno_fbcon;
    if (!fbcon)
        return -1;

    memset(fbcon, 0, sizeof(struct dev_fbcon));

    fbcon->buffer = u_malloc(FBCON_L * FBCON_H);
    if (!fbcon->buffer) {
        kfree(fbcon);
        return -1;
    }
    fbcon->colormap = u_malloc(FBCON_L * FBCON_H);
    if (!fbcon->colormap) {
        kfree(fbcon->buffer);
        kfree(fbcon);
        return -1;
    }

    if (devfs == NULL) {
        kfree(fbcon->colormap);
        kfree(fbcon->buffer);
        kfree(fbcon);
        return -1;
    }

    memset(fbcon->buffer, 0, (FBCON_L * FBCON_H));
    memset(fbcon->colormap, COLOR_DEFAULT, (FBCON_L * FBCON_H));
    register_module(&mod_devfbcon);
    fno_fbcon = fno_create(&mod_devfbcon, "fbcon", devfs);
    fno_fbcon->priv = fbcon;
    fbcon->size_x = FBCON_L;
    fbcon->size_y = FBCON_H;
    fbcon->screen = framebuffer_get();
    fbcon->color = COLOR_DEFAULT;
    framebuffer_setcmap(xterm_cmap);
    /* Test */
    devfbcon_write(fno_fbcon, color, 2);
    devfbcon_write(fno_fbcon, frosted_banner, strlen(frosted_banner));
    devfbcon_write(fno_fbcon, white, 2);
    devfbcon_write(fno_fbcon, fbcon_banner, strlen(fbcon_banner));
    return 0;
}
