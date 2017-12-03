#include "frosted.h"
#include "device.h"
#ifndef USBH_DRIVERS_H
#define USBH_DRIVERS_H

/* Put your driver initalizers here */

extern void usb_kbd_init(void);


static inline void usbh_drivers_init(void)
{
    #ifdef CONFIG_DEV_USBH_KBD
    usb_kbd_init();
    #endif
}


#endif
