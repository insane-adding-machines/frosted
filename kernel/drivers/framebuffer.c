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

#ifdef STM32F7
#   include "libopencm3/stm32/ltdc.h"
#endif

struct dev_fb {
    struct device * dev;
    uint32_t base;
};

static struct dev_fb fb0 = { 0 };

static int fb_write(struct fnode *fno, const void *buf, unsigned int len);
static int fb_read(struct fnode *fno, void *buf, unsigned int len);
static int fb_open(const char *path, int flags);

static struct module mod_devfb = {
    .family = FAMILY_FILE,
    .name = "framebuffer",
    .ops.open = fb_open,
    .ops.read = fb_read,
    .ops.write = fb_write,
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
    struct dev_fb *fb;

    if (len == 0)
        return len;

    fb = (struct dev_fb *)FNO_MOD_PRIV(fno, &mod_devfb);
    if (!fb)
        return -1;

    //frosted_mutex_lock(fb->dev->mutex);
    len_left = fno->size - fno->off;
    if (len > len_left)
        len = len_left;

    if (!len)
        return 0;

    /* write to framebuffer memory */
    memcpy((void *)((uint8_t *)fb->base + fno->off) , buf, len); 
    fno->off += len;

    //frosted_mutex_unlock(fb->dev->mutex);
    return len;
}


static int fb_read(struct fnode *fno, void *buf, unsigned int len)
{
    int len_left;
    struct dev_fb *fb;

    if (len == 0)
        return len;

    len_left = fno->size - fno->off;
    if (len > len_left)
        len = len_left;

    if (!len)
        return 0;

    fb = (struct dev_fb *)FNO_MOD_PRIV(fno, &mod_devfb);
    if (!fb)
        return -1;

    // frosted_mutex_lock(fb->dev->mutex);

    //XXX: max len = MAX_FB_SIZE;
    memcpy(buf, (void *)((uint8_t *)fb->base + fno->off), len);
    fno->off += len;

    //frosted_mutex_unlock(fb->dev->mutex);
    return len;
}

static int fb_fno_init(struct fnode *dev, int n, const struct fb_addr * addr)
{
    struct dev_fb *fb = &fb0;
    static int num_fb = 0;

    if (n != 0)
        return -1;

    char name[4] = "fb";
    name[2] =  '0' + num_fb++;

    fb->base = addr->base;
    fb->dev = device_fno_init(&mod_devfb, name, dev, FL_TTY, fb);

    fb->dev->fno->off = 0;
    fb->dev->fno->size = 480 * 272 * 2; // XXX HARDCODED for now
    return 0;

}

void fb_init(struct fnode * dev)
{
    /* init a single framebuffer for now */
    struct fb_addr fb_a;

    //uart_fno_init(dev, uart_addrs[i].devidx, &uart_addrs[i]);
    //CLOCK_ENABLE(uart_addrs[i].rcc);
    //usart_enable_rx_interrupt(uart_addrs[i].base);
    //usart_set_baudrate(uart_addrs[i].base, uart_addrs[i].baudrate);
    //usart_set_databits(uart_addrs[i].base, uart_addrs[i].data_bits);
    //usart_set_stopbits(uart_addrs[i].base, uart_addrs[i].stop_bits);
    //usart_set_mode(uart_addrs[i].base, USART_MODE_TX_RX);
    //usart_set_parity(uart_addrs[i].base, uart_addrs[i].parity);
    //usart_set_flow_control(uart_addrs[i].base, uart_addrs[i].flow);
    //usart_enable_rx_interrupt(uart_addrs[i].base);
    
    extern int stm32f7_ltdc_init(void);
    extern void stm32f7_ltdc_test(void);
    extern void * stm32f7_getfb(void);

    if (!stm32f7_ltdc_init())
        stm32f7_ltdc_test();

    fb_a.devidx = 0;
    fb_a.base = (uint32_t)stm32f7_getfb();

    fb_fno_init(dev, fb_a.devidx, &fb_a);

    register_module(&mod_devfb);
}
