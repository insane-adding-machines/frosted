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


static struct module mod_usb = {
    .family = FAMILY_DEV,
    .name = "usb-otg-guest",
};

static struct usbd_device *usbd_dev = NULL;

void otg_fs_isr(void)
{
    usbd_poll(usbd_dev, 0);
}

int usbdev_start(usbd_device **_usbd_dev,
          const struct usb_device_descriptor *dev_desc,
          void *buffer, size_t buffer_size)
{
    if (usbd_dev)
        return -EBUSY;

    usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, dev_desc, buffer,
                            buffer_size);
    *_usbd_dev = usbd_dev;
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    return 0;
}

int usb_init(struct usb_config *conf)
{
    gpio_create(&mod_usb, &conf->pio_vbus);
    gpio_create(&mod_usb, &conf->pio_dm);
    gpio_create(&mod_usb, &conf->pio_dp);
    return 0;
}
