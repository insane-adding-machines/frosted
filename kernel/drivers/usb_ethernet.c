/*
 *	USB Ethernet gadget driver
 *
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
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <pico_stack.h>
#include <pico_device.h>
#include <pico_ipv4.h>

#define USBETH_MAX_FRAME 1514
struct pico_dev_usbeth {
    struct pico_device dev;
    usbd_device *usbd_dev;
    int tx_busy;
};

static struct pico_dev_usbeth *pico_usbeth = NULL;
static const struct usb_device_descriptor usbdev_desc= {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_CLASS_CDC,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

static const struct usb_endpoint_descriptor comm_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x82,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 0x10,
} };

static const struct usb_endpoint_descriptor data_endp[] = {{
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x01,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = 64,
    .bInterval = 1,
}, {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x81,
    .bmAttributes = USB_ENDPOINT_ATTR_BULK,
    .wMaxPacketSize = 64,
    .bInterval = 1,
} };

static const struct {
    struct usb_cdc_header_descriptor header;
    struct usb_cdc_union_descriptor cdc_union;
    struct usb_cdc_ecm_descriptor ecm;
} __attribute__((packed)) cdcecm_functional_descriptors = {
    .header = {
        .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
        .bcdCDC = 0x0120,
    },
    .cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_UNION,
        .bControlInterface = 0,
        .bSubordinateInterface0 = 1,
     },
    .ecm = {
        .bFunctionLength = sizeof(struct usb_cdc_ecm_descriptor),
        .bDescriptorType = CS_INTERFACE,
        .bDescriptorSubtype = USB_CDC_TYPE_ECM,
        .iMACAddress = 4,
        .bmEthernetStatistics = { 0, 0, 0, 0 },
        .wMaxSegmentSize = USBETH_MAX_FRAME,
        .wNumberMCFilters = 0,
        .bNumberPowerFilters = 0,
    },
};

static const struct usb_interface_descriptor comm_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ECM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
    .iInterface = 0,

    .endpoint = comm_endp,

    .extra = &cdcecm_functional_descriptors,
    .extralen = sizeof(cdcecm_functional_descriptors)
} };

static const struct usb_interface_descriptor data_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
    .endpoint = data_endp,
} };

static const struct usb_interface ifaces[] = {
    {
        .num_altsetting = 1,
        .altsetting = comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting = data_iface,
    }
};

static const struct usb_config_descriptor config = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 71,
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,

    .interface = ifaces,
};

const char usb_string_manuf[] = "Insane adding machines";
const char usb_string_name[] = "Frosted Eth gadget";
const char usb_serialn[] = "01";
const char usb_macaddr[] = "005af341b4c9";


static const char *usb_strings[4] = {
    usb_string_manuf, usb_string_name, usb_serialn, usb_macaddr
};

static const uint8_t mac_addr[6] = { 0, 0x5a, 0xf3, 0x41, 0xb4, 0xca };

static int cdcecm_control_request(usbd_device *usbd_dev,
    struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest) {
        case USB_CDC_REQ_SET_ETHERNET_MULTICAST_FILTER:
        case USB_CDC_REQ_SET_ETHERNET_PACKET_FILTER:
        case USB_CDC_REQ_SET_ETHERNET_PM_PATTERN_FILTER:
            return 1;
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
        return 1;
        }
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding)) {
            return 0;
        }

        return 1;
    }
    return 0;
}


struct usbeth_rx_buffer {
    uint16_t size;
    int status;
    uint8_t buf[USBETH_MAX_FRAME];
};

static struct usbeth_rx_buffer *rx_buffer = NULL;
#define RXBUF_FREE 0
#define RXBUF_INCOMING 1
#define RXBUF_TCPIP    2
static void rx_buffer_free(uint8_t *arg)
{
    (void) arg;
    rx_buffer->size = 0;
    rx_buffer->status = RXBUF_FREE;
}

struct usbeth_tx_frame {
    uint16_t off;
    uint16_t size;
    uint8_t *base;
};

static struct usbeth_tx_frame tx_frame = {};

static void cdcecm_data_tx_complete_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    int ret;
    int len;
    if (pico_usbeth->tx_busy == 0)
        return;
    len = tx_frame.size - tx_frame.off;
    if (len > 64)
        len = 64;

    tx_frame.off += usbd_ep_write_packet(pico_usbeth->usbd_dev, 0x81, tx_frame.base + tx_frame.off, len);
    if (tx_frame.off == tx_frame.size)
        pico_usbeth->tx_busy = 0;
}

static void cdcecm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    int len;
    if (!rx_buffer) {
        rx_buffer = kalloc(USBETH_MAX_FRAME);
        rx_buffer->size =0;
        rx_buffer->status = RXBUF_FREE; /* First call! */
    }
    if (!pico_usbeth || !rx_buffer || (rx_buffer->status != RXBUF_FREE)) {
        char buf[64];
        len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);
        (void)len;
        return;
    }

    len = usbd_ep_read_packet(usbd_dev, 0x01, rx_buffer->buf + rx_buffer->size, 64);
    if (len > 0) {
        rx_buffer->size += len;
    }
    if (len < 64) {
        /* End of frame. */
        rx_buffer->status++; /* incoming packet */
    }
}

static void cdcecm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    (void)wValue;

    usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64,
            cdcecm_data_rx_cb);
    usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, 64, cdcecm_data_tx_complete_cb);
    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(
                usbd_dev,
                USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                cdcecm_control_request);
}


static int pico_usbeth_send(struct pico_device *dev, void *buf, int len)
{
    struct pico_dev_usbeth *usbeth = (struct pico_dev_usbeth *) dev;
    int ret;
    if (pico_usbeth->tx_busy > 0)
        return 0;

    if ((tx_frame.size > 0) && (tx_frame.size == tx_frame.off) && (tx_frame.base == buf)) {
        memset(&tx_frame, 0, sizeof(tx_frame));
        return len;
    }

    if (len <= 64)
        return usbd_ep_write_packet(usbeth->usbd_dev, 0x81, buf, len);

    tx_frame.base = buf;
    tx_frame.size = len;
    tx_frame.off = usbd_ep_write_packet(usbeth->usbd_dev, 0x81, buf, 64);
    pico_usbeth->tx_busy++;
    return 0;
}

static int pico_usbeth_poll(struct pico_device *dev, int loop_score)
{
    static int notified_link_up = 0;
    if (!notified_link_up) {
        struct pico_dev_usbeth *usbeth = (struct pico_dev_usbeth *) dev;
        uint8_t buf[8] = { };
        buf[0] = 0x51;
        buf[1] = 0;
        buf[2] = 1;
        buf[3] = 0;

        notified_link_up++;
        usbd_ep_write_packet(usbeth->usbd_dev, 0x82, buf, 8);
    }
    if (rx_buffer->status == RXBUF_INCOMING) {
        pico_stack_recv_zerocopy_ext_buffer_notify(&pico_usbeth->dev, rx_buffer->buf, rx_buffer->size, rx_buffer_free);
        rx_buffer->status++;
        loop_score--;
    }
    return loop_score;
}

static void pico_usbeth_destroy(struct pico_device *dev)
{
    struct pico_dev_usbeth *usbeth = (struct pico_dev_usbeth *) dev;
    kfree(rx_buffer);
    kfree(usbeth);
    pico_usbeth = NULL;
}

void otg_fs_isr(void)
{
    if (pico_usbeth)
        usbd_poll(pico_usbeth->usbd_dev);
}


int usb_ethernet_init(void)
{
    struct pico_dev_usbeth *usb = kalloc(sizeof(struct pico_dev_usbeth));;
    uint8_t *usb_buf;
    struct pico_ip4 default_ip, default_nm, default_gw, zero;
    const char ipstr[] = CONFIG_USB_DEFAULT_IP;
    const char nmstr[] = CONFIG_USB_DEFAULT_NM;
    const char gwstr[] = CONFIG_USB_DEFAULT_GW;


    memset(&zero, 0, sizeof(zero));


    if (!usb)
        return -ENOMEM;

    usb_buf = kalloc(128);
    if (!usb_buf) {
        kfree(usb);
        return -ENOMEM;
    }

    pico_string_to_ipv4(ipstr, &default_ip.addr);
    pico_string_to_ipv4(nmstr, &default_nm.addr);
    pico_string_to_ipv4(gwstr, &default_gw.addr);


    memset(usb, 0, sizeof(struct pico_dev_usbeth));
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_OTGFS);

    usb->dev.overhead = 0;
    usb->dev.send = pico_usbeth_send;
    usb->dev.poll = pico_usbeth_poll;
    usb->dev.destroy = pico_usbeth_destroy;
    if (pico_device_init(&usb->dev,"usb0", mac_addr) < 0) {
        kfree(usb_buf);
        kfree(usb);
        return -1;
    }
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO9 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);

    /* USB init */
    usb->usbd_dev = usbd_init(&otgfs_usb_driver, &usbdev_desc, &config,
            usb_strings, 4,
            usb_buf, 128);
    usbd_register_set_config_callback(usb->usbd_dev, cdcecm_set_config);
    pico_usbeth = usb;

    /* Set address/netmask */
    pico_ipv4_link_add(&usb->dev, default_ip, default_nm);
    /* Set default gateway */
    pico_ipv4_route_add(zero, zero, default_gw, 1, NULL);
    nvic_enable_irq(NVIC_OTG_FS_IRQ);
    pico_usbeth->tx_busy = 0;
    return 0;
}
