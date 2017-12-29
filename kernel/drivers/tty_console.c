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
#include "framebuffer.h"

#define KBD_PATH "/dev/kbd0"
#define FBCON_PATH "/dev/fbcon"

#define KBD_MOD "usbkbd"
#define FBCON_MOD "fbcon"


static struct tty_console {
    struct device *dev;
    struct fnode *kbd;
    struct fnode *fbcon;
    struct module *mod_kbd;
    struct module *mod_fbcon;
    uint16_t pid;
} TTY;

static int tty_read(struct fnode *fno, void *buf, unsigned int len);
static int tty_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static int tty_write(struct fnode *fno, const void *buf, unsigned int len);
static int tty_seek(struct fnode *fno, int off, int whence);
static void tty_attach(struct fnode *fno, int pid);

static struct module mod_ttycon = {
    .family = FAMILY_DEV,
    .name = "tty",
    .ops.open = device_open,
    .ops.read = tty_read,
    .ops.poll = tty_poll,
    .ops.write = tty_write,
    .ops.seek = tty_seek,
    .ops.tty_attach = tty_attach
};

static void devfile_create(void) 
{
    char name[5] = "tty";
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return;
    TTY.dev = device_fno_init(&mod_ttycon, name, devfs, FL_TTY, &TTY);
}

int tty_console_init(void)
{
    TTY.mod_kbd = module_search(KBD_MOD);
    TTY.mod_fbcon = module_search(FBCON_MOD);
    if(!TTY.mod_kbd || !TTY.mod_fbcon)
        return -1;
    TTY.kbd = fno_search(KBD_PATH);
    TTY.fbcon = fno_search(FBCON_PATH);
    devfile_create();
    return 0;
}

static void tty_send_break(void *arg)
{
    int *pid = (int *)(arg);
    if (pid)
        task_kill(*pid, 2);
}

static int tty_read(struct fnode *fno, void *buf, unsigned int len)
{
    int ret, i;
    if (!TTY.kbd)
        TTY.kbd = fno_search(KBD_PATH);
    if (!TTY.kbd)
        return 0;
    ret = TTY.mod_kbd->ops.read(TTY.kbd, buf, len);
    if (TTY.pid > 1) {
        for (i = 0; i < ret; i++) {
            if (((uint8_t *)buf)[i] == 0x03) /* Ctrl + c*/
                tasklet_add(tty_send_break, &TTY.pid); 
        }
    }
    return ret;
}

static int tty_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    if (!TTY.kbd)
        TTY.kbd = fno_search(KBD_PATH);
    if (!TTY.kbd)
        return 0;
    return TTY.mod_kbd->ops.poll(TTY.kbd, events, revents);
}

static int tty_write(struct fnode *fno, const void *buf, unsigned int len)
{
    if (!TTY.fbcon)
        TTY.fbcon = fno_search(FBCON_PATH);
    return TTY.mod_fbcon->ops.write(TTY.fbcon, buf, len);
}

static int tty_seek(struct fnode *fno, int off, int whence)
{
    if (!TTY.fbcon)
        TTY.fbcon = fno_search(FBCON_PATH);
    if (!TTY.fbcon)
        return -ENOENT;
    return TTY.mod_fbcon->ops.seek(TTY.fbcon, off, whence);
}

static void tty_attach(struct fnode *fno, int pid)
{
    if (!TTY.fbcon)
        TTY.fbcon = fno_search(FBCON_PATH);
    if (!TTY.fbcon)
        return;
    TTY.mod_fbcon->ops.tty_attach(TTY.fbcon, pid);
    TTY.pid = pid;
}

