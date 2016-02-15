#include "frosted.h"
#include "device.h"
#ifdef STM32F4
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
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

static struct module mod_devusb = {
    .family = FAMILY_FILE,
    .name = "usb",
    .ops.open = device_open,
};

void otg_fs_isr(void)
{
    struct dev_usb *u = &DEV_USB[0];

    if (u)
        usbd_poll(u->usbd_dev);
}



usbd_device * usb_register_set_config_callback(struct fnode *fno, usbd_set_config_callback callback)
{
    struct dev_usb *usb = NULL;
    
    usb = (struct dev_usb *)FNO_MOD_PRIV(fno, &mod_devusb);
    
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



static void usb_fno_init(struct fnode *dev, uint32_t n, const struct usb_addr * addr)
{

    usbd_device *usbd_dev;


    struct dev_usb *a = &DEV_USB[n];
    a->dev = device_fno_init(&mod_devusb, addr->name, dev, FL_RDONLY, a);
    a->irq = addr->irq;
    a->usb_buf = kalloc(128);
    a->num_callbacks = addr->num_callbacks;



    usbd_dev = usbd_init(    &otgfs_usb_driver, 
                                                    addr->usbdev_desc, 
                                                    addr->config,
                                                    addr->usb_strings, 
                                                    addr->num_usb_strings,
                                                    a->usb_buf, 
                                                    128);
    

    a->usbd_dev = usbd_dev;

}


void usb_init(struct fnode *dev, const struct usb_addr usb_addrs[], int num_usb)
{
    int i;
    for (i = 0; i < num_usb; i++) 
    {
        rcc_periph_clock_enable(usb_addrs[i].rcc);
        usb_fno_init(dev, i, &usb_addrs[i]);
    }
}

