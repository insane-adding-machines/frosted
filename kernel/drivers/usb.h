#ifndef INC_USB
#define INC_USB
#include "frosted.h"
#include "gpio.h"
#include <unicore-mx/usbd/usbd.h>
#include <unicore-mx/usb/class/cdc.h>
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/cm3/nvic.h>
#include <unicore-mx/usbd/misc/string.h>

#define USB_MODE_GUEST 1
#define USB_MODE_HOST  2

struct usb_config {
    unsigned int otg_mode;
    struct gpio_config pio_vbus, pio_dm, pio_dp;
};

#ifdef CONFIG_DEVUSB
    int usb_init(struct usb_config *conf);
    int usbdev_start(usbd_device **_usbd_dev,
          const struct usb_device_descriptor *dev_desc);
#else
#  define usb_init(x) ((-ENOENT))
#  define usbdev_start(x,y) ((-ENOENT))
#endif


#ifdef CONFIG_DEV_USBETH
    int usb_ethernet_init(void);
#else
#  define usb_ethernet_init() ((-ENOENT))
#endif

#endif
