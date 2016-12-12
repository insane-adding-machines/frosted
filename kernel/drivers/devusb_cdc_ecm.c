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
#include <pico_stack.h>
#include <pico_device.h>
#include <pico_ipv4.h>
#include "usb.h"

#define USBETH_MAX_FRAME 1514
struct pico_dev_usbeth {
    struct pico_device dev;

    enum {
        TX_STATUS_FREE,
        TX_STATUS_PENDING,
        TX_STATUS_COMPLETE
    } tx_status;
};

struct usbeth_rx_buffer {
    enum usbeth_rx_buffer_status {
        RX_STATUS_FREE, /**< Buffer is free */
        RX_STATUS_INCOMING, /**< We have unprocessed buffer */
        RX_STATUS_TCPIP /** PICO TCP is processing */
    } status;

    uint16_t size;
    uint8_t buf[USBETH_MAX_FRAME];
};

static struct pico_dev_usbeth *pico_usbeth = NULL;
static struct usbeth_rx_buffer *rx_buffer = NULL;

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
    .extra_len = sizeof(cdcecm_functional_descriptors)
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

static const char usb_string_manuf[] = "Insane adding machines";
static const char usb_string_name[] = "Frosted Eth gadget";
static const char usb_serialn[] = "01";
static const char usb_macaddr[] = "005af341b4c9";


static const char *usb_strings_ascii[4] = {
    usb_string_manuf, usb_string_name, usb_serialn, usb_macaddr
};

static const struct usb_string_utf8_data usb_strings[] = {{
    .data = (const uint8_t **) usb_strings_ascii,
    .count = 4,
    .lang_id = USB_LANGID_ENGLISH_UNITED_STATES
}, {
    .data = NULL
}};

static const struct usb_config_descriptor cdc_ecm_config = {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 71,
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,

    .interface = ifaces,
    .string = usb_strings
};

static const struct usb_device_descriptor cdc_ecm_dev = {
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

    .config = &cdc_ecm_config,
    .string = usb_strings
};

static const uint8_t mac_addr[6] = { 0, 0x5a, 0xf3, 0x41, 0xb4, 0xca };

uint8_t usbdev_buffer[128] __attribute__((aligned(16)));

static void cdcecm_setup_request(usbd_device *usbd_dev, uint8_t ep_addr,
            const struct usb_setup_data *setup_data)
{
    (void) ep_addr; /* Assuming (ep_addr == 0) */

    const uint8_t bmReqMask = USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT;
    const uint8_t bmReqVal = USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE;

    if ((setup_data->bmRequestType & bmReqMask) != bmReqVal) {
        /* Not of our interest, let library handle */
        usbd_ep0_setup(usbd_dev, setup_data);
        return;
    }

    switch (setup_data->bRequest) {
    case USB_CDC_REQ_SET_ETHERNET_MULTICAST_FILTER:
    case USB_CDC_REQ_SET_ETHERNET_PACKET_FILTER:
    case USB_CDC_REQ_SET_ETHERNET_PM_PATTERN_FILTER:
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        usbd_ep0_transfer(usbd_dev, setup_data, NULL, 0, NULL);
    return;
    case USB_CDC_REQ_SET_LINE_CODING:
        if (setup_data->wLength < sizeof(struct usb_cdc_line_coding)) {
            break;
        }

        /* Just read whatever the host is sending */
        usbd_ep0_transfer(usbd_dev, setup_data, usbdev_buffer,
                setup_data->wLength, NULL);
    return;
    }

    /* Unsupported request */
    usbd_ep0_stall(usbd_dev);
}




/***************************
 *                         *
 *  USB Device Definition  *
 *                         *
 ***************************
 *
 *
 */
static void cdcecm_set_config(usbd_device *usbd_dev,
        const struct usb_config_descriptor *cfg);

static usbd_device *usbd_dev;

static void notify_link_up(void)
{
    static const uint8_t buf[8] = {0x51, 0, 1, 0, 0, 0, 0, 0};
    const usbd_transfer transfer = {
        .ep_type = USBD_EP_INTERRUPT,
        .ep_addr = 0x82,
        .ep_size = 16,
        .ep_interval = 0,
        .buffer = (void *) buf,
        .length = sizeof(buf),
        .flags = USBD_FLAG_NONE,
        .timeout = USBD_TIMEOUT_NEVER,
        .callback = NULL
    };

    usbd_transfer_submit(usbd_dev, &transfer);
}

static void bulk_out_callback(usbd_device *dev,
            const usbd_transfer *transfer, usbd_transfer_status status,
            usbd_urb_id urb_id)
{
    static int notified_link_up = 0;
    static int fail_count = 0;
    (void)urb_id;

    if (status != USBD_SUCCESS) {
        return;
    }

    if (!notified_link_up) {
        notified_link_up = 1;
        notify_link_up();
    }

    if (rx_buffer->status == RX_STATUS_FREE) {
        rx_buffer->size = transfer->transferred;
        rx_buffer->status = RX_STATUS_INCOMING;
    } else {
        fail_count++;
    }
    frosted_tcpip_wakeup();
}

static void bulk_out_submit(void)
{
    rx_buffer->size = 0;
    rx_buffer->status = RX_STATUS_FREE;

    const usbd_transfer transfer = {
        .ep_type = USBD_EP_BULK,
        .ep_addr = 0x01,
        .ep_size = 64,
        .ep_interval = USBD_INTERVAL_NA,
        .buffer = rx_buffer->buf,
        .length = USBETH_MAX_FRAME,
        .flags = USBD_FLAG_SHORT_PACKET,
        .timeout = USBD_TIMEOUT_NEVER,
        .callback = bulk_out_callback
    };

    usbd_transfer_submit(usbd_dev, &transfer);
}

static void cdcecm_set_config(usbd_device *usbd_dev,
        const struct usb_config_descriptor *cfg)
{
    (void)cfg;

    usbd_ep_prepare(usbd_dev, 0x01, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
    usbd_ep_prepare(usbd_dev, 0x81, USBD_EP_BULK, 64, USBD_INTERVAL_NA, USBD_EP_NONE);
    usbd_ep_prepare(usbd_dev, 0x82, USBD_EP_INTERRUPT, 16, 0, USBD_EP_NONE);

    if (!rx_buffer) {
        rx_buffer = kalloc(sizeof(*rx_buffer));
    }

    bulk_out_submit();
}

/**
 * Transfer callback for BULK IN (TX)
 */
static void bulk_in_callback(usbd_device *dev,
    const usbd_transfer *transfer, usbd_transfer_status status,
    usbd_urb_id urb_id)
{
    (void)dev;
    (void)transfer;
    (void)urb_id;

    pico_usbeth->tx_status = TX_STATUS_COMPLETE;
    frosted_tcpip_wakeup();
}

static int pico_usbeth_send(struct pico_device *dev, void *buf, int len)
{
    struct pico_dev_usbeth *usbeth = (struct pico_dev_usbeth *) dev;
    static int fail_count = 0;
    const usbd_transfer transfer = {
        .ep_type = USBD_EP_BULK,
        .ep_addr = 0x81,
        .ep_size = 64,
        .ep_interval = USBD_INTERVAL_NA,
        .buffer = buf,
        .length = len,
        .flags = USBD_FLAG_NONE,
        .timeout = USBD_TIMEOUT_NEVER,
        .callback = bulk_in_callback
    };

    if (pico_usbeth->tx_status == TX_STATUS_COMPLETE) {
        pico_usbeth->tx_status = TX_STATUS_FREE;
        return len;
    } else if (pico_usbeth->tx_status == TX_STATUS_PENDING) {
        fail_count++;
        return 0;
    }
    usbd_transfer_submit(usbd_dev, &transfer);
    pico_usbeth->tx_status = TX_STATUS_PENDING;
    return 0;
}

static int pico_usbeth_poll(struct pico_device *dev, int loop_score)
{
    if (rx_buffer->status == RX_STATUS_INCOMING) {
        pico_stack_recv(&pico_usbeth->dev, rx_buffer->buf, rx_buffer->size);
        bulk_out_submit();
        loop_score--;
    }
    return loop_score;
}

static void pico_usbeth_destroy(struct pico_device *dev)
{
    struct pico_dev_usbeth *usbeth = (struct pico_dev_usbeth *) dev;
    kfree(rx_buffer);
    kfree(usbeth);
    usbeth = NULL;
    rx_buffer = NULL;
}

int usb_ethernet_init(void)
{
    struct pico_dev_usbeth *usb = kalloc(sizeof(*usb));
    uint8_t *usb_buf;
    struct pico_ip4 default_ip, default_nm, default_gw, zero;
    const char ipstr[] = CONFIG_USB_DEFAULT_IP;
    const char nmstr[] = CONFIG_USB_DEFAULT_NM;
    const char gwstr[] = CONFIG_USB_DEFAULT_GW;
    zero.addr = 0U;


    pico_string_to_ipv4(ipstr, &default_ip.addr);
    pico_string_to_ipv4(nmstr, &default_nm.addr);
    pico_string_to_ipv4(gwstr, &default_gw.addr);
    memset(usb, 0, sizeof(struct pico_dev_usbeth));

    usb->dev.overhead = 0;
    usb->dev.send = pico_usbeth_send;
    usb->dev.poll = pico_usbeth_poll;
    usb->dev.destroy = pico_usbeth_destroy;
    if (pico_device_init(&usb->dev,"usb0", mac_addr) < 0) {
        kfree(usb_buf);
        kfree(usb);
        return -1;
    }

    pico_usbeth = usb;
    /* Set address/netmask */
    pico_ipv4_link_add(&usb->dev, default_ip, default_nm);
    /* Set default gateway */
    pico_ipv4_route_add(zero, zero, default_gw, 1, NULL);
    pico_usbeth->tx_status = TX_STATUS_FREE;
    if (usbdev_start(&usbd_dev, &cdc_ecm_dev, usbdev_buffer,
                                    sizeof(usbdev_buffer)) < 0) {
        return -EBUSY;
    }

    usbd_register_set_config_callback(usbd_dev, cdcecm_set_config);
    usbd_register_setup_callback(usbd_dev, cdcecm_setup_request);

    return 0;
}
