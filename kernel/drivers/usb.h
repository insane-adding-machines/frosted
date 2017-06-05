#ifndef INC_USB
#define INC_USB
#include "frosted.h"
#include "gpio.h"
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/usbh/usbh.h>

#define USB_DEV_FS 0
#define USB_DEV_HS 1

#define USB_MODE_GUEST 1
#define USB_MODE_HOST  2

typedef void (*usb_host_driver_probe_callback)(
        struct usbh_device *dev,
        const struct usb_device_descriptor *device_desc,
        const struct usb_config_descriptor *config_desc);

typedef void (*usb_host_interface_removed_callback)(
    struct usbh_device *dev, uint8_t bInterfaceNumber);

int usb_host_driver_register(struct module *owner,
        usb_host_driver_probe_callback probe);

int usb_host_claim_interface(struct module *owner,
        usbh_device *dev, uint8_t bInterfaceNumber,
        usb_host_interface_removed_callback removed);

int usb_host_release_interface(usbh_device *dev,
        uint8_t bInterfaceNumber);

struct usb_pio_config_fs {
    struct gpio_config pio_vbus;
    struct gpio_config pio_dm;
    struct gpio_config pio_dp;
};

struct usb_pio_config_hs {
    struct gpio_config ulpi_data[8];
    struct gpio_config ulpi_clk;
    struct gpio_config ulpi_dir;
    struct gpio_config ulpi_next;
    struct gpio_config ulpi_step;
};

struct usb_config {
    unsigned int dev_type;
    unsigned int otg_mode;

    union usb_pio_config {
        struct usb_pio_config_fs *fs;
        struct usb_pio_config_hs *hs;
    } pio;
};

#ifdef CONFIG_DEVUSB
    int usb_init(struct usb_config *conf);
    int usbdev_start(usbd_device **_usbd_dev, unsigned int dev,
          const struct usbd_info *info);
#else
#  define usb_init(x) ((-ENOENT))
#  define usbdev_start(...) ((-ENOENT))
#endif


#ifdef CONFIG_DEV_USBETH
    int usb_ethernet_init(unsigned int dev);
#else
#  define usb_ethernet_init(x) ((-ENOENT))
#endif

#endif
