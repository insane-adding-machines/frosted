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
#include <unicore-mx/usbh/usbh.h>
#include <unicore-mx/usbh/helper/ctrlreq.h>
#include <unicore-mx/usbh/class/hid.h>


static struct module mod_usb_guest = {
    .family = FAMILY_DEV,
    .name = "usb-otg-guest",
};

static struct module mod_usb_host = {
    .family = FAMILY_DEV,
    .name = "usb-otg-host",
};

/* High-speed backend config */
static const usbd_backend_config hs_backend_config = {
    .ep_count = 9,
    .priv_mem = 4096,
    .speed = USBH_SPEED_HIGH,
    .feature = USBH_PHY_EXT
};


static struct usbd_device *usbd_dev = NULL;
static usbh_host *usbh_dev = NULL;

void otg_fs_isr(void)
{
    if (usbd_dev)
        usbd_poll(usbd_dev, 0);
}

void otg_hs_isr(void)
{
    if (usbd_dev)
        usbd_poll(usbd_dev, 0);
}

static void kthread_usbhost(void *arg)
{
    (void)arg;
    kthread_sleep_ms(1000);
    while(1) {
        usbh_poll(usbh_dev, 0);
        kthread_yield();
    }
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
        usbd_dev = usbd_init(USBD_STM32_OTG_HS, &hs_backend_config, dev_desc, buffer,
                                buffer_size);
        nvic_enable_irq(NVIC_OTG_HS_IRQ);
    }
    
    *_usbd_dev = usbd_dev;
    return 0;
}

#ifdef CONFIG_USBHOST

struct usb_driver {
    struct module *owner;
    int (*probe)(struct usb_interface_descriptor *iface);
    int (*register_endpoint)(struct usb_endpoint_descriptor *ep);
    void (*boot)(const usbh_transfer *transfer, usbh_transfer_status status, usbh_urb_id urb_id);
    struct usb_driver *next;
};

static struct usb_driver *UsbDriverList = NULL;

static int usb_driver_register(struct module *owner, 
        int (*probe)(struct usb_interface_descriptor *iface), 
        int (*register_endpoint)(struct usb_endpoint_descriptor *ep),
        void (*boot)(const usbh_transfer *transfer, usbh_transfer_status status, usbh_urb_id urb_id) 
        )
{
    struct usb_driver *usb = kalloc(sizeof(struct usb_driver));
    if (!usb)
        return -ENOMEM;

    if (!probe || !register_endpoint)
        return -EINVAL;

    usb->probe = probe;
    usb->register_endpoint = register_endpoint;
    usb->boot = boot;
    usb->next = UsbDriverList;
    UsbDriverList = usb;
    return 0;
}

static struct usb_driver *usb_driver_probe(struct usb_interface_descriptor *iface)
{
    struct usb_driver *usb = UsbDriverList;
    while(usb) {
        if (usb->probe(iface) == 0)
            return usb;
        usb = usb->next;
    }
    return NULL;
}
static void usbh_dev_disconnected(usbh_device *dev)
{

}

static uint8_t usbh_dev_ifnum;
static uint8_t usbh_dev_altset;
static struct usb_driver *usb;

static void usbh_config_set(const usbh_transfer *transfer, usbh_transfer_status status, usbh_urb_id urb_id)
{
    (void) urb_id;
    if (status != USBH_SUCCESS)
        return;
    usbh_ctrlreq_set_interface(transfer->device, usbh_dev_ifnum, usbh_dev_altset, usb->boot);
    usb = NULL;
}

static void usbh_conf_described(const usbh_transfer *transfer, usbh_transfer_status status, usbh_urb_id urb_id)
{
    struct usb_config_descriptor *cfg = transfer->data;
    int len = cfg->wTotalLength - cfg->bLength;
    void *ptr = transfer->data + cfg->bLength;
    struct usb_interface_descriptor *iface_interest = NULL;
    (void) urb_id;

    if (status != USBH_SUCCESS) {
        return;
    }


    if (!transfer->transferred || cfg->bLength < 4) {
        /* Descriptor too small */
        return;
    }

    if (cfg->wTotalLength > transfer->transferred) {
        /* Descriptor too long to be read in one shot */
        return;
    }


    while (len > 0) {
        uint8_t *d = ptr;
        switch (d[1]) {
            case USB_DT_INTERFACE: 
                {
                    struct usb_interface_descriptor *iface = ptr;
                    
                    /* If the interesting interface was already found, don't visit
                     * the endpoints of other configurations
                     */
                    if (iface_interest && (iface != iface_interest))
                        return;

                    usb = usb_driver_probe(iface);
                    if (usb) {
                        iface_interest = iface;
                        usbh_dev_ifnum = iface->bInterfaceNumber;
                        usbh_dev_altset = iface->bAlternateSetting;
                    } else {
                        iface_interest = NULL;
                    }
                } 
                break;

            case USB_DT_ENDPOINT: 
                {
                    struct usb_endpoint_descriptor *ep = ptr;
                    if (usb && iface_interest) { 
                        if ( usb->register_endpoint(ep) == 0) {
                            usbh_ctrlreq_set_config(transfer->device, cfg->bConfigurationValue, usbh_config_set);
                        }
                    }
                } 
                break;
        }

        ptr += d[0];
        len -= d[0];
    }

}

static uint8_t buf[128];
void usbh_dev_described(const usbh_transfer *transfer, usbh_transfer_status status, usbh_urb_id urb_id)
{
    struct usb_device_descriptor *desc = transfer->data;
    uint8_t class = desc->bDeviceClass;
    uint8_t subclass = desc->bDeviceSubClass;
    uint8_t protocol = desc->bDeviceProtocol;

    (void) urb_id;

    if (status != USBH_SUCCESS) {
        return;
    }


    if (!desc->bNumConfigurations) {
        return;
    }

    usbh_device *dev = transfer->device;
    usbh_ctrlreq_read_desc(dev, USB_DT_CONFIGURATION, 0, buf, 128, usbh_conf_described);
}

static void usbh_device_plugged(usbh_device *dev)
{
    usb = NULL;
    usbh_device_register_disconnected_callback(dev, usbh_dev_disconnected);
    usbh_ctrlreq_read_desc(dev, USB_DT_DEVICE, 0, buf, 18, usbh_dev_described);
}

/* High-speed backend config */
static const usbh_backend_config hs_host_backend_config = {
    .chan_count = 12,
    .priv_mem = 4096,
    .speed = USBH_SPEED_HIGH,
    /* VBUS_EXT = PHY IC drive VBUS */
    .feature = USBH_PHY_EXT | USBH_VBUS_EXT
};

static void usbhost_start(void)
{
    if (usbh_dev)
        return ;
#ifdef CONFIG_USBFSHOST
	usbh_dev = usbh_init(USBH_STM32_OTG_FS, NULL);
	usbh_register_connected_callback(usbh_dev, usbh_device_plugged);
	nvic_enable_irq(NVIC_OTG_FS_IRQ);
#else
	usbh_dev = usbh_init(USBH_STM32_OTG_HS, &hs_host_backend_config);
	usbh_register_connected_callback(usbh_dev, usbh_device_plugged);
	nvic_enable_irq(NVIC_OTG_HS_IRQ);
#endif
    kthread_create(kthread_usbhost, NULL);
}
#endif

int usb_init(struct usb_config *conf)
{
    struct module *mod = &mod_usb_guest;

    if (conf->otg_mode == USB_MODE_HOST) {
        mod = &mod_usb_host;
    }
    if (conf->dev_type == USB_DEV_FS) {
        gpio_create(mod, &conf->pio.fs->pio_vbus);
        gpio_create(mod, &conf->pio.fs->pio_dm);
        gpio_create(mod, &conf->pio.fs->pio_dp);
    } else if (conf->dev_type == USB_DEV_HS) {
        int i = 0;
        gpio_create(mod, &conf->pio.hs->ulpi_clk);
        for (i = 0; i < 8; i++)
            gpio_create(mod, &conf->pio.hs->ulpi_data[i]);
        gpio_create(mod, &conf->pio.hs->ulpi_step);
        gpio_create(mod, &conf->pio.hs->ulpi_dir);
        gpio_create(mod, &conf->pio.hs->ulpi_next);
    } else {
        return -1;
    }

#ifdef CONFIG_USBHOST
    if (conf->otg_mode == USB_MODE_HOST) {
        usbhost_start();
    }
#endif
    return 0;
}
