#include "frosted.h"
#include "device.h"

#include "cdc_ecm.h"
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <pico_stack.h>
#include <pico_device.h>
#include <pico_ipv4.h>
#ifdef STM32F4
#include "stm32f4_usb.h"
#endif

struct pico_dev_usbeth {
    struct pico_device dev;
    usbd_device *usbd_dev;
    int tx_busy;
};

static struct pico_dev_usbeth *pico_usbeth = NULL;

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

    usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcecm_data_rx_cb);
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


void cdcecm_init(const unsigned char * usb_name)
{
    struct fnode *fno;
    struct pico_ip4 default_ip, default_nm;
    struct pico_dev_usbeth *usb = kalloc(sizeof(struct pico_dev_usbeth));;

    if (!usb)
        return;

    memset(usb, 0, sizeof(struct pico_dev_usbeth));
    fno = fno_search(usb_name);

    pico_string_to_ipv4(CONFIG_USB_DEFAULT_IP, &default_ip.addr);
    pico_string_to_ipv4(CONFIG_USB_DEFAULT_NM, &default_nm.addr);

    usb->dev.overhead = 0;
    usb->dev.send = pico_usbeth_send;
    usb->dev.poll = pico_usbeth_poll;
    usb->dev.destroy = pico_usbeth_destroy;
    if (pico_device_init(&usb->dev,"usb0", mac_addr) < 0) {
        kfree(usb);
    }
    else
    {
        usb->usbd_dev =  usb_register_set_config_callback(fno, cdcecm_set_config);
        pico_usbeth = usb;
        
        pico_ipv4_link_add(&usb->dev, default_ip, default_nm);
        
        pico_usbeth->tx_busy = 0;
    }
}

