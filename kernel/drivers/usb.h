#ifndef INC_USB
#define INC_USB
#include "frosted.h"
#include "gpio.h"
#include <unicore-mx/usb/usbd.h>
#include <unicore-mx/usb/cdc.h>
#include <unicore-mx/cm3/scb.h>
#include <unicore-mx/cm3/nvic.h>

#define USB_MODE_GUEST 1
#define USB_MODE_HOST  2

struct usb_config {
    unsigned int otg_mode;
    struct gpio_config pio_vbus, pio_dm, pio_dp;
};


#define DEVUSB_BUFLEN 128
#define DEVUSB_STRINGS 128
struct devusb_config {
    usbd_device *usbd_dev;
    const struct usb_device_descriptor *dev_desc;
    const struct usb_config_descriptor *conf_desc;
    const struct usb_endpoint_descriptor * data_endp;
    const char ** strings;
    int n_strings;
    uint8_t  buffer[DEVUSB_BUFLEN];
    void (*callback)(usbd_device *usbd_dev, uint16_t wValue);
};

#ifdef CONFIG_DEVUSB
    int usb_init(struct usb_config *conf);
    int usbdev_start(struct devusb_config *conf);
#else
#  define usb_init(x) ((-ENOENT))
#  define usbdev_start(x) ((-ENOENT))
#endif


#ifdef CONFIG_DEVUSB_ETH
    int usb_ethernet_init(void);
#else
#  define usb_ethernet_init() ((-ENOENT))
#endif

#endif
