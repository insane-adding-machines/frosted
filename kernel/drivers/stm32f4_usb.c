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
#ifdef STM32F4
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/usb/usbd.h>
#endif
#include "stm32f4_usb.h"

struct dev_usb{
    struct device * dev;
    uint32_t irq;
    usbd_device *usbd_dev;
    const struct usb_endpoint_descriptor * data_endp;
    uint8_t *usb_buf;
    uint8_t num_callbacks;
};

#define MAX_USBS 1

static struct dev_usb DEV_USB[MAX_USBS];

void otg_fs_isr(void)
{
    struct dev_usb *u = &DEV_USB[0];

    if (u)
        usbd_poll(u->usbd_dev);
}

usbd_device * usb_register_set_config_callback(usbd_set_config_callback callback)
{
    struct dev_usb *usb = &DEV_USB[0];
    if (usb == NULL)
        return NULL;
    if (usb->usbd_dev == NULL)
        return NULL;

    if(usb->num_callbacks>0)
    {
        if(usbd_register_set_config_callback(usb->usbd_dev, callback) != 0)
        {
            return NULL;
        }

        if(usb->num_callbacks == 1)
        {
            nvic_enable_irq(usb->irq);
        }
        usb->num_callbacks--;
    }
    return usb->usbd_dev;
}


void usb_init(const struct usb_addr usb_addrs[], int num_usb)
{
    int i;
    for (i = 0; i < num_usb; i++)
    {
        rcc_periph_clock_enable(usb_addrs[i].rcc);
    }
}
