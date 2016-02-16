#ifndef INC_USB
#define INC_USB

struct usb_addr{
    uint32_t irq;
    uint32_t rcc;
    uint8_t num_callbacks;
    const char * name;
    const struct usb_device_descriptor * usbdev_desc;
    const struct usb_config_descriptor * config;
    const char ** usb_strings;
    int num_usb_strings;
};

void usb_init(struct fnode *dev, const struct usb_addr usb_addrs[], int num_usb);
usbd_device * usb_register_set_config_callback(struct fnode *fno, usbd_set_config_callback callback);

#endif

