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
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/usbd/usbd.h>
#include "usb.h"
#include "gpio.h"

static struct module mod_usb = {
    .family = FAMILY_DEV,
    .name = "usb-otg-guest",
};



#define MAX_USBS 1

static struct devusb_config *dev_usb = NULL;

void otg_fs_isr(void)
{
    usbd_poll(dev_usb->usbd_dev);
}


int usbdev_start(struct devusb_config *d)
{
    if (dev_usb)
        return -EBUSY;

    rcc_periph_clock_enable(RCC_OTGFS);
    dev_usb = d;
    dev_usb->usbd_dev = usbd_init(&otgfs_usb_driver, dev_usb->dev_desc, dev_usb->conf_desc,
            dev_usb->strings, 4,
            dev_usb->buffer, 128);
    usbd_register_set_config_callback(dev_usb->usbd_dev, dev_usb->callback);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
}

int usb_init(struct usb_config *conf)
{
    int i;
    gpio_create(&mod_usb, &conf->pio_vbus);
    gpio_create(&mod_usb, &conf->pio_dm);
    gpio_create(&mod_usb, &conf->pio_dp);
    return 0;
}
