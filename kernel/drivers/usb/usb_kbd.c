#include "frosted.h"
#include "device.h"
#include "usb.h"
#include <unicore-mx/usbh/usbh.h>
#include <unicore-mx/usbh/helper/ctrlreq.h>
#include <unicore-mx/usbh/class/hid.h>


#define CLASS_HID 0x03
#define SUBCLASS_HID_BOOT 0x01
#define PROTOCOL_HID_KEYBOARD 0x01
#define KBD_BUF_SIZE 128

#ifndef FALL_THROUGH
#define FALL_THROUGH do{}while(0)
#endif

static int kbd_read(struct fnode *fno, void *buf, unsigned int len);
static int kbd_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static void usb_kbd_disconnect(struct usbh_device *dev, uint8_t bInterfaceNumber);

static struct module mod_usbkbd = {
    .family = FAMILY_DEV,
    .name = "usbkbd",
    .ops.open = device_open,
    .ops.read = kbd_read,
    .ops.poll = kbd_poll,
};

/* Only support one keyboard for now. 
 * Extend this to an array or list to 
 * claim multiple keyboards.
 */

struct keyboard {
    uint8_t iface_num, 
            iface_altset, 
            ep_number, 
            ep_size, 
            ep_interval;

    uint8_t buf[KBD_BUF_SIZE];
} KBD;

static void kbd_data_in(const uint8_t *data, unsigned len)
{

}

static void endpoint_read_cb (const usbh_transfer *transfer,
	usbh_transfer_status status, usbh_urb_id urb_id)
{
	(void) urb_id;
    /* TODO: add a default for unexpected error. Delete device. */

	switch (status) {
	case USBH_SUCCESS:
		kbd_data_in(transfer->data, transfer->transferred);
        FALL_THROUGH;
	case USBH_ERR_TIMEOUT:
        FALL_THROUGH;
	case USBH_ERR_IO:
		/* resubmit! */
		usbh_transfer_submit(transfer);
        break;
	}
}

static void read_data_from_keyboard(usbh_device *dev)
{
	usbh_transfer ep_transfer = {
		.device = dev,
		.ep_type = USBH_EP_INTERRUPT,
		.ep_addr = KBD.ep_number,
		.ep_size = KBD.ep_size,
		.interval = KBD.ep_interval,
		.data = KBD.buf,
		.length = KBD.ep_size,
		.flags = USBH_FLAG_NONE,
		.timeout = 250,
		.callback = endpoint_read_cb,
	};
	usbh_transfer_submit(&ep_transfer);
}

static void after_set_idle(const usbh_transfer *transfer,
	usbh_transfer_status status, usbh_urb_id urb_id)
{
	(void) urb_id;
    (void) status;
	read_data_from_keyboard(transfer->device);
}

static void after_set_protocol_boot(const usbh_transfer *transfer,
	usbh_transfer_status status, usbh_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBH_SUCCESS) {
		return;
	}
    usbh_hid_set_idle(transfer->device, 0, 0, KBD.iface_num, after_set_idle);
}

static void after_interface_set(const usbh_transfer *transfer,
	usbh_transfer_status status, usbh_urb_id urb_id)
{
	(void) urb_id;
    (void) status;
	usbh_hid_set_protocol(transfer->device, USB_REQ_HID_PROTOCOL_BOOT,
		KBD.iface_num, after_set_protocol_boot);
}

static void after_config_set(const usbh_transfer *transfer,
	usbh_transfer_status status, usbh_urb_id urb_id)
{
	(void) urb_id;

	if (status != USBH_SUCCESS) {
		return;
	}
	usbh_ctrlreq_set_interface(transfer->device, KBD.iface_num,
			KBD.iface_altset, after_interface_set);
}

static void usb_kbd_probe( struct usbh_device *dev,
        const struct usb_device_descriptor *ddesc,
        const struct usb_config_descriptor *cfg)
{
    uint8_t class = ddesc->bDeviceClass;
    uint8_t subclass = ddesc->bDeviceSubClass;
    uint8_t protocol = ddesc->bDeviceProtocol;
    int ifnum = -1;
	uint8_t  *ptr = ((uint8_t *)cfg) + cfg->bLength;
	int len = cfg->wTotalLength - cfg->bLength;
	struct usb_interface_descriptor *iface_interest = NULL;
	struct usb_endpoint_descriptor *ep_interest = NULL;

    /* Check device configurations */
    if (ddesc->bNumConfigurations < 1)
        return;

    /* Check class, subclass, protocol for direct claim */
    if ((class == CLASS_HID) || (subclass == SUBCLASS_HID_BOOT) || (protocol == PROTOCOL_HID_KEYBOARD)) {
        /* TODO: claim */

        return;
    }
         

    if (cfg->bLength < 4) {
        /* Descriptor is too small. */
        return;
    }

	while (len > 0) {
        switch (ptr[1]) {
            case USB_DT_INTERFACE: 
                {
                    struct usb_interface_descriptor *iface = 
                        (struct usb_interface_descriptor *)ptr;
                    if (iface->bInterfaceClass == CLASS_HID &&
                            iface->bInterfaceSubClass == SUBCLASS_HID_BOOT &&
                            iface->bInterfaceProtocol == PROTOCOL_HID_KEYBOARD &&
                            iface->bNumEndpoints) {
                        iface_interest = iface;
                    }

                    ep_interest = NULL;
                } break;
            case USB_DT_ENDPOINT: 
                {
                    struct usb_endpoint_descriptor *ep = (struct usb_endpoint_descriptor *)ptr;
                    if (iface_interest == NULL) {
                        break;
                    }
                    if ((ep->bmAttributes & USB_ENDPOINT_ATTR_TYPE) ==
                            USB_ENDPOINT_ATTR_INTERRUPT &&
                            (ep->bEndpointAddress & 0x80)) {
                        ep_interest = ep;
                    }
                } break;
        } /* end switch/case */

		if ((iface_interest != NULL) && (ep_interest != NULL)) {
			break;
		}
		ptr += ptr[0];
		len -= ptr[0];
	}

    if ((iface_interest == NULL) || (ep_interest == NULL)) {
        return;
    }
	KBD.iface_num = iface_interest->bInterfaceNumber;
	KBD.iface_altset = iface_interest->bAlternateSetting;
	KBD.ep_number = ep_interest->bEndpointAddress;
	KBD.ep_size = ep_interest->wMaxPacketSize;
	KBD.ep_interval = ep_interest->bInterval;
    if (usb_host_claim_interface(&mod_usbkbd, dev, KBD.iface_num, usb_kbd_disconnect) < 0) {
        /* Failed to claim interface. */
        return;
    }

	usbh_ctrlreq_set_config(dev, cfg->bConfigurationValue,
			after_config_set);
}

static void usb_kbd_disconnect(struct usbh_device *dev, uint8_t bInterfaceNumber)
{
    /* TODO: delete char device. */
    return; 
}


void usb_kbd_init(void) 
{
    register_module(&mod_usbkbd);
    usb_host_driver_register(&mod_usbkbd, usb_kbd_probe);
}

static int kbd_read(struct fnode *fno, void *buf, unsigned int len) 
{
    return -1;
}

static int kbd_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    return 0;
}
