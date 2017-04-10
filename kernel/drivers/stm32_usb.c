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
#include "usb.h"
#include "gpio.h"
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/usbd/usbd.h>


static struct module mod_usb_guest = {
    .family = FAMILY_DEV,
    .name = "usb-otg-guest",
};

static struct module mod_usb_host = {
    .family = FAMILY_DEV,
    .name = "usb-otg-host",
};


static struct usbd_device *usbd_dev = NULL;
static struct usbh_device *usbh_dev = NULL;

void otg_fs_isr(void)
{
    usbd_poll(usbd_dev, 0);
}

void otg_hs_isr(void)
{
    usbd_poll(usbd_dev, 0);
}

int usbdev_start(usbd_device **_usbd_dev, unsigned int dev,
          const struct usb_device_descriptor *dev_desc,
          void *buffer, size_t buffer_size)
{
    if (usbd_dev)
        return -EBUSY;

    if (dev == USB_DEV_FS) {
        usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, dev_desc, buffer,
                                buffer_size);
        nvic_enable_irq(NVIC_OTG_FS_IRQ);
    } else {
        usbd_dev = usbd_init(USBD_STM32_OTG_HS, NULL, dev_desc, buffer,
                                buffer_size);
        nvic_enable_irq(NVIC_OTG_HS_IRQ);
    }
    
    *_usbd_dev = usbd_dev;
    return 0;
}

int usb_init(struct usb_config *conf)
{
    struct module *mod = &mod_usb_guest;
    if (conf->otg_mode == USB_MODE_HOST) {
        mod = &mod_usb_host;
    }
    if (conf->dev_type == USB_DEV_FS) {
        rcc_periph_clock_enable(RCC_OTGFS);
        gpio_create(mod, &conf->pio.fs->pio_vbus);
        gpio_create(mod, &conf->pio.fs->pio_dm);
        gpio_create(mod, &conf->pio.fs->pio_dp);
    } else if (conf->dev_type == USB_DEV_HS) {
        int i = 0;
        rcc_periph_clock_enable(RCC_OTGHS);
        rcc_periph_clock_enable(RCC_OTGHSULPI);
        for (i = 0; i < 8; i++)
            gpio_create(mod, &conf->pio.hs->ulpi_data[i]);
        gpio_create(mod, &conf->pio.hs->ulpi_clk);
        gpio_create(mod, &conf->pio.hs->ulpi_dir);
        gpio_create(mod, &conf->pio.hs->ulpi_next);
        gpio_create(mod, &conf->pio.hs->ulpi_step);
    } else {
        return -1;
    }
    return 0;
}
