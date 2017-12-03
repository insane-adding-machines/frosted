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
#include "usb/usbh_drivers.h"
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/stm32/rcc.h>
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usbh/usbh.h>
#include <unicore-mx/usbh/helper/ctrlreq.h>
#include <unicore-mx/usbh/class/hid.h>
#include <stdbool.h>


static struct module mod_usb_guest = {
    .family = FAMILY_DEV,
    .name = "usb-otg-guest",
};

static struct module mod_usb_host = {
    .family = FAMILY_DEV,
    .name = "usb-otg-host",
};

/* Device: High-speed backend config */
static const usbd_backend_config hs_dev_backend_config = {
    .ep_count = 9,
    .priv_mem = 4096,
    .speed = USBH_SPEED_HIGH,
    .feature = USBH_PHY_EXT
};

/* Host: High-speed backend config */
static const usbh_backend_config hs_host_backend_config = {
    .chan_count = 12,
    .priv_mem = 4096,
    .speed = USBH_SPEED_HIGH,
    /* VBUS_EXT = PHY IC drive VBUS */
    .feature = USBH_PHY_EXT | USBH_VBUS_EXT
};

static struct usbd_device *_usbd_dev = NULL;
static usbh_host *_usbh_host = NULL;

void otg_fs_isr(void)
{
    if (_usbd_dev)
        usbd_poll(_usbd_dev, 0);
}

void otg_hs_isr(void)
{
    if (_usbd_dev)
        usbd_poll(_usbd_dev, 0);
}

static void kthread_usbhost(void *arg)
{
    (void)arg;
    volatile uint32_t last = jiffies;

    while(1) {
        if (last != jiffies) {
            usbh_poll(_usbh_host, (jiffies - last) * 1000);
            last = jiffies;
        }
        //kthread_yield();
    }
}

int usbdev_start(usbd_device **usbd_dev, unsigned int dev,
          const struct usbd_info *info)
{
    if (_usbd_dev)
        return -EBUSY;

    if (dev == USB_DEV_FS) {
        _usbd_dev = usbd_init(USBD_STM32_OTG_FS, NULL, info);
        nvic_enable_irq(NVIC_OTG_FS_IRQ);
    } else {
        _usbd_dev = usbd_init(USBD_STM32_OTG_HS, &hs_dev_backend_config, info);
        nvic_enable_irq(NVIC_OTG_HS_IRQ);
    }

    *usbd_dev = _usbd_dev;
    return 0;
}

#ifdef CONFIG_USBHOST

struct usb_host_driver {
    struct module *owner;
    usb_host_driver_probe_callback probe;
    struct usb_host_driver *next;
};

struct usb_host_claim {
    struct module *owner;

    /* Reason of call:
     *  - Device disconnected
     *  - Configuration change
     *  - Interface released
     */
    usb_host_interface_removed_callback removed;

    usbh_device *device;

    /* Interface number */
    uint8_t bInterfaceNumber;

    struct usb_host_claim *next;
};

static struct usb_host_driver *HOST_DRIVER_LIST = NULL;
static struct usb_host_claim *HOST_CLAIM_LIST = NULL;

int usb_host_driver_register(struct module *owner,
        usb_host_driver_probe_callback probe)
{
    if (!probe) {
        return -EINVAL;
    }

    struct usb_host_driver *driver = kalloc(sizeof(struct usb_host_driver));
    if (!driver) {
        return -ENOMEM;
    }

    driver->probe = probe;
    driver->next = HOST_DRIVER_LIST;
    HOST_DRIVER_LIST = driver;
    return 0;
}

/**
 * Check weather the interface has already been claimed
 * @param dev Device (USB Host)
 * @param bInterfaceNumber Interface number
 * @return true on claimed, false on still open for claiming
 */
bool interface_claimed(usbh_device *dev, uint8_t bInterfaceNumber)
{
    const struct usb_host_claim *claim;

    for (claim = HOST_CLAIM_LIST; claim; claim = claim->next) {
        if (claim->device == dev && claim->bInterfaceNumber == bInterfaceNumber) {
            return true;
        }
    }

    return false;
}

/**
 * Claim the USB Host device interface.
 * @param owner Owner
 * @param dev USB Host device
 * @param bInterfaceNumber Interface number
 * @param removed Callback when removed.
 * @return 0 on success
 * @return -EBUSY Interface already claimed
 * @return -EINVAL Invalid value passed
 * @note Make sure @a bInterfaceNumber is a valid interface number
 *  for the device and currently selected configuration.
 * @note Driver need to manually SET_INTEFACE
 */
int usb_host_claim_interface(struct module *owner,
        usbh_device *dev, uint8_t bInterfaceNumber,
        usb_host_interface_removed_callback removed)
{
    if (!dev || !removed) {
        return -EINVAL;
    }

    if (interface_claimed(dev, bInterfaceNumber)) {
        return -EBUSY;
    }

    struct usb_host_claim *claim = kalloc(sizeof(struct usb_host_claim));
    if (!claim) {
        return -ENOMEM;
    }

    claim->owner = owner;
    claim->removed = removed;
    claim->bInterfaceNumber = bInterfaceNumber;
    claim->next = HOST_CLAIM_LIST;
    HOST_CLAIM_LIST = claim;
    return 0;
}

/**
 * Release a previously claimed interface
 * @param dev USB Host - device
 * @param bInterfaceNumber Interface number
 * @return 0 success
 * @return -EINVAL Invalid argument
 * @return -ENOENT No such entry
 */
int usb_host_release_interface(usbh_device *dev, uint8_t bInterfaceNumber)
{
    struct usb_host_claim *claim = HOST_CLAIM_LIST, *tmp, *prev = NULL;

    if (!dev) {
        return -EINVAL;
    }

    while (claim != NULL) {
        if (claim->device != dev) {
            prev = claim;
            claim = claim->next;
            continue;
        }

        /* Update link */
        if (prev == NULL) { /* First item */
            HOST_CLAIM_LIST = claim->next;
        } else {
            prev->next = claim->next;
        }

        /* Goto next entry */
        tmp = claim;
        claim = claim->next;

        /* Remove the item */
        tmp->removed(tmp->device, tmp->bInterfaceNumber);
        kfree(tmp);

        return 0;
    }

    return -ENOENT;
}

/**
 * USB Host - device disconnected.
 * Remove any claimed interface from the list for the given device
 * @param dev USB Host device
 */
static void host_dev_disconnected_callback(usbh_device *dev)
{
    struct usb_host_claim *claim = HOST_CLAIM_LIST, *tmp, *prev = NULL;

    while (claim != NULL) {
        if (claim->device != dev) {
            prev = claim;
            claim = claim->next;
            continue;
        }

        /* Update link */
        if (prev == NULL) { /* First item */
            HOST_CLAIM_LIST = claim->next;
        } else {
            prev->next = claim->next;
        }

        /* Goto next entry */
        tmp = claim;
        claim = claim->next;

        /* Remove the item */
        tmp->removed(tmp->device, tmp->bInterfaceNumber);
        kfree(tmp);
    }
}

static uint8_t buf_dev_desc[18];
static uint8_t buf_config_desc[128];

static void host_set_config_callback(const usbh_transfer *transfer,
                usbh_transfer_status status, usbh_urb_id urb_id)
{
    (void) urb_id;

    const struct usb_host_driver *usb;

    const struct usb_device_descriptor *dev_desc = (void *) buf_dev_desc;
    const struct usb_config_descriptor *config_desc = (void *) buf_config_desc;

    if (status != USBH_SUCCESS) {
        return;
    }

    /* Let each driver probe the configuration descriptor and
     * react accordingly */
    for (usb = HOST_DRIVER_LIST; usb; usb = usb->next) {
        usb->probe(transfer->device, dev_desc, config_desc);
    }
}

static void host_config_desc_read_callback(const usbh_transfer *transfer,
                usbh_transfer_status status, usbh_urb_id urb_id)
{
    (void) urb_id;

    const struct usb_config_descriptor *cfg = transfer->data;

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

    /* The descriptor look good. SET_CONFIGURATION before probing */
    usbh_ctrlreq_set_config(transfer->device, cfg->bConfigurationValue,
                        host_set_config_callback);
}

static void host_dev_desc_read_callback(const usbh_transfer *transfer,
                usbh_transfer_status status, usbh_urb_id urb_id)
{
    const struct usb_device_descriptor *desc = transfer->data;

    (void) urb_id;

    if (status != USBH_SUCCESS) {
        return;
    }

    if (!desc->bNumConfigurations) {
        return;
    }

    /* Read the index=0 configuration descriptor */
    usbh_ctrlreq_read_desc(transfer->device, USB_DT_CONFIGURATION, 0,
                buf_config_desc, sizeof(buf_config_desc), host_config_desc_read_callback);
}

static void host_dev_connected_callback(usbh_device *dev)
{
    usbh_device_register_disconnected_callback(dev, host_dev_disconnected_callback);
    usbh_ctrlreq_read_desc(dev, USB_DT_DEVICE, 0, buf_dev_desc,
                sizeof(buf_dev_desc), host_dev_desc_read_callback);
}

static void usbhost_start(void)
{
    if (_usbh_host)
        return ;
#ifdef CONFIG_USBFSHOST
    _usbh_host = usbh_init(USBH_STM32_OTG_FS, NULL);
    usbh_register_connected_callback(_usbh_host, host_dev_connected_callback);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
#else /* CONFIG_USBFSHOST */
    _usbh_host = usbh_init(USBH_STM32_OTG_HS, &hs_host_backend_config);
    usbh_register_connected_callback(_usbh_host, host_dev_connected_callback);
    nvic_enable_irq(NVIC_OTG_HS_IRQ);
#endif /* CONFIG_USBFSHOST */
    usbh_drivers_init();
    kthread_create(kthread_usbhost, NULL);
}
#endif /* CONFIG_USBHOST */

int usb_init(struct usb_config *conf)
{
    struct module *mod = &mod_usb_guest;

    if (conf->otg_mode == USB_MODE_HOST) {
        mod = &mod_usb_host;
        if (conf->dev_type == USB_DEV_FS) {
            gpio_create(mod, &conf->pio.fs->pio_phy);
            gpio_clear(conf->pio.fs->pio_phy.base, conf->pio.fs->pio_phy.pin);
        }
    }
    if (conf->dev_type == USB_DEV_FS) {
        gpio_create(mod, &conf->pio.fs->pio_vbus);
        gpio_create(mod, &conf->pio.fs->pio_dm);
        gpio_create(mod, &conf->pio.fs->pio_dp);
        gpio_create(mod, &conf->pio.fs->pio_phy);
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
