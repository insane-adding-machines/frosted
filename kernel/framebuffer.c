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
#include "framebuffer.h"
#include <stdint.h>
#include <sys/ioctl.h>

#ifdef STM32F7
#   include "unicore-mx/stm32/ltdc.h"
#endif


#define MAX_FBS         (1)


static struct fb_info *fb[MAX_FBS] = { 0 };

static int fb_write(struct fnode *fno, const void *buf, unsigned int len);
static int fb_read(struct fnode *fno, void *buf, unsigned int len);
static int fb_open(const char *path, int flags);
static int fb_seek(struct fnode *fno, int off, int whence);
static int fb_ioctl(struct fnode * fno, const uint32_t cmd, void *arg);

static struct module mod_devfb = {
    .family = FAMILY_FILE,
    .name = "framebuffer",
    .ops.open = fb_open,
    .ops.read = fb_read,
    .ops.write = fb_write,
    .ops.seek = fb_seek,
    .ops.ioctl = fb_ioctl,
};


static int fb_open(const char *path, int flags)
{
    struct fnode *f = fno_search(path);
    if (!f)
        return -1;
    f->off = 0;
    return device_open(path, flags);
}

static int fb_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int len_left;
    struct fb_info *fb;

    if (len == 0)
        return len;

    fb = (struct fb_info *)FNO_MOD_PRIV(fno, &mod_devfb);
    if (!fb)
        return -1;

    //mutex_lock(fb->dev->mutex);
    len_left = fno->size - fno->off;
    if (len > len_left)
        len = len_left;

    if (!len)
        return 0;

    /* write to framebuffer memory */
    memcpy((void *)((uint8_t *)fb->screen_buffer + fno->off) , buf, len); 
    fno->off += len;

    //mutex_unlock(fb->dev->mutex);
    return len;
}


static int fb_read(struct fnode *fno, void *buf, unsigned int len)
{
    int len_left;
    struct fb_info *fb;

    if (len == 0)
        return len;

    len_left = fno->size - fno->off;
    if (len > len_left)
        len = len_left;

    if (!len)
        return 0;

    fb = (struct fb_info *)FNO_MOD_PRIV(fno, &mod_devfb);
    if (!fb)
        return -1;

    // mutex_lock(fb->dev->mutex);

    //XXX: max len = MAX_FB_SIZE;
    memcpy(buf, (void *)((uint8_t *)fb->screen_buffer + fno->off), len);
    fno->off += len;

    //mutex_unlock(fb->dev->mutex);
    return len;
}

/* TODO: Could probably be made generic ? */
static int fb_seek(struct fnode *fno, int off, int whence)
{
    struct fb_info *fb;
    int new_off;

    switch(whence) {
        case SEEK_CUR:
            new_off = fno->off + off;
            break;
        case SEEK_SET:
            new_off = off;
            break;
        case SEEK_END:
            new_off = fno->size + off;
            break;
        default:
            return -1;
    }

    if (new_off < 0)
        new_off = 0;

    if (new_off > fno->size) {
        return -1;
    }
    fno->off = new_off;
    return fno->off;
}


static int fb_fno_init(struct fnode *dev, struct fb_info * fb)
{
    static int num_fb = 0;
    char name[4] = "fb";

    if (!fb)
        return -1;

    name[2] =  '0' + num_fb++;

    fb->dev = device_fno_init(&mod_devfb, name, dev, FL_TTY, fb);

    fb->dev->fno->off = 0;
    fb->dev->fno->size = fb->var.yres * fb->var.xres * fb->var.bits_per_pixel;
    return 0;
}

static int fb_ioctl(struct fnode * fno, const uint32_t cmd, void *arg)
{
    struct fb_info * fb = (struct fb_info *)FNO_MOD_PRIV(fno, &mod_devfb);
    if (!fb)
        return -1;

    (void)arg;
    if (cmd == IOCTL_FB_GETCMAP) {
        //return exti_enable(fno, 0);
        return 0;
    }
    if (cmd == IOCTL_FB_PUTCMAP) {
        return fb->fbops->fb_setcmap((struct fb_cmap *)(arg), fb);
    }
    return -1;
}

unsigned char *framebuffer_get(void)
{
    return fb[0]->screen_buffer;
}

int fb_init(void)
{
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return -ENOENT;
    /* Ony one FB supported for now */
    fb_fno_init(devfs, fb[0]);
    return 0;
}

/* Register a low-level framebuffer driver */
int register_framebuffer(struct fb_info *fb_info)
{
    if (!fb_info)
        return -1;
    
    if (!fb_info->fbops)
        return -1;

    if (fb_info->fbops->fb_open)
        fb_info->fbops->fb_open(fb_info);

    if (fb_info->fbops->fb_blank)
        fb_info->fbops->fb_blank(fb_info);

    fb[0] = fb_info;

    register_module(&mod_devfb);

    return 0;
}
