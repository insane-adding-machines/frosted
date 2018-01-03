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
 *      Authors: Daniele Lacamera
 *
 */
#include "frosted.h"
#include "device.h"
#include "usb.h"
#include "cirbuf.h"
#include "poll.h"
#include <unicore-mx/usbh/usbh.h>
#include <unicore-mx/usbh/helper/ctrlreq.h>
#include <unicore-mx/usbh/class/hid.h>

#define CLASS_HID 0x03
#define SUBCLASS_HID_BOOT 0x01
#define PROTOCOL_HID_KEYBOARD 0x01
#define KBD_BUF_SIZE 128
#define KBD_FIFO_SIZE 16

/* Special keys */

#define CAPS_LOCK 0x39
#define SHIFT_L   0x02
#define SHIFT_R   0x20
#define CTRL_L    0x01
#define CTRL_R    0x10

#define ESC       0x1B
#define BS        0x08
#define DEL       0x7F

const char map_num[10][2] = {
    {'1', '!'},
    {'2', '@'},
    {'3', '#'},
    {'4', '$'},
    {'5', '%'},
    {'6', '^'},
    {'7', '&'},
    {'8', '*'},
    {'9', '('},
    {'0', ')'}
};

const char map_others[17][2] = {
    {'\n', '\r'},
    {ESC, ESC},
    {BS, DEL},
    {'\t','\t'},
    {' ', ' '},
    {'-', '_'},
    {'=', '+'},
    {'[', '{'},
    {']', '}'},
    {'\\', '|'},
    {0, 0},
    {';', ':'},
    {'\'', '"'},
    {'`', '~'},
    {',', '<'},
    {'.', '>'},
    {'/', '?'},
};

const char map_arrows[4] = { 'C', 'D', 'B', 'A' };



#ifndef FALL_THROUGH
#define FALL_THROUGH do{}while(0)
#endif

static int kbd_read(struct fnode *fno, void *buf, unsigned int len);
static int kbd_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static int kbd_ioctl(struct fnode *fno, const uint32_t cmd, void *arg);
static void usb_kbd_disconnect(struct usbh_device *dev, uint8_t bInterfaceNumber);

static struct module mod_usbkbd = {
    .family = FAMILY_DEV,
    .name = "usbkbd",
    .ops.open = device_open,
    .ops.read = kbd_read,
    .ops.ioctl = kbd_ioctl,
    .ops.poll = kbd_poll,
};

/* Only support one keyboard for now. 
 * Extend this to an array or list to 
 * claim multiple keyboards.
 */

static struct keyboard {
    struct device *dev;
    uint8_t iface_num, 
            iface_altset, 
            ep_number, 
            ep_size, 
            ep_interval;
    uint8_t buf[KBD_BUF_SIZE];
    struct  cirbuf *cfifo;
    int     caps_lock;
    uint8_t     mode;
} KBD;


static void kbd_data_in(const uint8_t *data, unsigned len)
{
    char mod = 0;
    int i;
    char shift;
    char c;
    int special = 0;
    int escape = 0;

    if (KBD.mode == K_RAW) {
        cirbuf_writebytes(KBD.cfifo, data, len);
        return;
    }

    if (len <= 0)
        return;
    mod = data[0];
    for (i = 0; i < len -2; i++) {
        uint8_t u = data[i + 2];
        c = '\0';
        if (u == CAPS_LOCK) {
            KBD.caps_lock = !KBD.caps_lock;
            continue;
        }
        if ((mod & SHIFT_L) || (mod & SHIFT_R)) {
            shift = (KBD.caps_lock)?'a':'A';
            special = 1;
        } else {
            shift = (KBD.caps_lock)?'A':'a';
            special = 0;
        }
        
        /* Intercept CTRL */
        if ((mod & CTRL_L) || (mod & CTRL_R)) {
            if (u == 0x06) /* CTRL + C */
                c = 0x03;
            continue;
        }

        /* Letters */
        if ((u >= 4) && (u <= 0x1d))
            c = (u - 4) + shift;

        /* Numbers and co. */
        else if ((u >= 0x1e) && (u <= 0x27))
            c = map_num[u - 0x1e][special];

        /* Other printable symbols */
        else if ((u >= 0x28) && (u <= 0x38))
            c = map_others[u - 0x28][special];
        
        /* Arrow keys */
        if ((u >= 0x4f) && (u <= 0x52)) {
            escape = 1;
            c = map_arrows[u - 0x4f];
        }

        /* load result in buffer */
        if (c) {
            mutex_lock(KBD.dev->mutex);
            if (escape) {
                cirbuf_writebyte(KBD.cfifo, ESC);
                cirbuf_writebyte(KBD.cfifo, '[');
            }
            cirbuf_writebyte(KBD.cfifo, c);
            task_resume(KBD.dev->task);
            mutex_unlock(KBD.dev->mutex);
        }
    }
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



static void devfile_create(void) 
{
    char name[5] = "kbd0";
    struct fnode *devfs = fno_search("/dev");
    if (!devfs)
        return;
    KBD.dev = device_fno_init(&mod_usbkbd, name, devfs, FL_TTY, &KBD);
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
    KBD.cfifo = cirbuf_create(KBD_FIFO_SIZE);
    devfile_create();
    
}

static void usb_kbd_disconnect(struct usbh_device *dev, uint8_t bInterfaceNumber)
{
    /* TODO: delete char device. */
    return; 
}

void usb_kbd_init(void) 
{
    KBD.mode = K_XLATE; 
    register_module(&mod_usbkbd);
    usb_host_driver_register(&mod_usbkbd, usb_kbd_probe);
}

static int kbd_read(struct fnode *fno, void *buf, unsigned int len) 
{
    int ret = 0;
    char *cbuf = (char *)buf;
    if (len == 0)
        return 0;
    KBD.dev->task = this_task();
    mutex_lock(KBD.dev->mutex);
    if (cirbuf_bytesinuse(KBD.cfifo) > 0) {
        ret = cirbuf_readbytes(KBD.cfifo, buf, len);
    } else {
        task_suspend();
        ret = SYS_CALL_AGAIN;
    }
    mutex_unlock(KBD.dev->mutex);
    return ret;
}

static int kbd_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    int ret = 0;
    KBD.dev->task = this_task();
    mutex_lock(KBD.dev->mutex);
    *revents = 0;
    if ((events == POLLIN) && (cirbuf_bytesinuse(KBD.cfifo) > 0)) {
        *revents |= POLLIN;
        ret = 1;
    }
    mutex_unlock(KBD.dev->mutex);
    return ret;
}

static int kbd_ioctl(struct fnode *fno, const uint32_t cmd, void *arg)
{
    if (cmd == KDSKBMODE) {
        mutex_lock(KBD.dev->mutex);
        uint32_t *mode = (uint32_t *)arg;
        uint8_t newmode = (*mode) & 0xFF;
        uint8_t c;
        if (newmode != KBD.mode) {
            while(cirbuf_bytesinuse(KBD.cfifo) > 0)
                cirbuf_readbyte(KBD.cfifo, &c);
            KBD.mode = newmode;
        }
        mutex_unlock(KBD.dev->mutex);
    } else {
        return -EINVAL; 
    }
}
