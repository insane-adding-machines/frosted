#include "frosted.h"
#include "device.h"

#include "cdc_acm.h"
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#ifdef STM32F4
#include "stm32f4_usb.h"
#endif




struct dev_cdcacm{
    struct device * dev;
    struct fnode *usb_fnode;
    usbd_device *usbd_dev;
    struct cirbuf *inbuf;
    struct cirbuf *outbuf;
    uint8_t *w_start;
    uint8_t *w_end;
    int tx_busy;
};

#define MAX_CDCACMS 1

static struct dev_cdcacm DEV_CDCACM[MAX_CDCACMS];

static int devcdcacm_write(struct fnode *fno, const void *buf, unsigned int len);
static int devcdcacm_read(struct fnode *fno, void *buf, unsigned int len);
static int devcdcacm_poll(struct fnode *fno, uint16_t events, uint16_t *revents);
static int devcdcacm_open(const char *path, int flags);

static struct module mod_devcdcacm = {
    .family = FAMILY_FILE,
    .name = "cdcacm",
    .ops.open = devcdcacm_open,
    .ops.read = devcdcacm_read, 
    .ops.poll = devcdcacm_poll,
    .ops.write = devcdcacm_write,
};


static int devcdcacm_open(const char *path, int flags)
{
    device_open(path, flags);
}



int glue_set_line_coding_cb(uint32_t baud, uint8_t databits,
			    enum usb_cdc_line_coding_bParityType cdc_parity,
			    enum usb_cdc_line_coding_bCharFormat cdc_stopbits)
{
    return 1;
}

void glue_set_line_state_cb(uint8_t dtr, uint8_t rts)
{
}

static int devcdcacm_write(struct fnode *fno, const void *buf, unsigned int len)
{
    int i;
    uint8_t ubuf[64];
    char *ch = (char *)buf;
    struct dev_cdcacm *cdcacm;

    cdcacm = (struct dev_cdcacm *)FNO_MOD_PRIV(fno, &mod_devcdcacm);
    if (!cdcacm)
        return -1;
    
    if (len <= 0)
        return len;

    if(cdcacm->tx_busy > 0)
        return 0;

    if (len <= 64)
        return usbd_ep_write_packet(cdcacm->usbd_dev, 0x82, buf, len);


    if (cdcacm->w_start == NULL) {
        cdcacm->w_start = (uint8_t *)buf;
        cdcacm->w_end = ((uint8_t *)buf) + len;

    } else {
        /* previous transmit not finished, do not update w_start */
    }

    cdcacm->tx_busy++;

    frosted_mutex_lock(cdcacm->dev->mutex);

    /* write to circular output buffer */
    cdcacm->w_start += cirbuf_writebytes(cdcacm->outbuf, cdcacm->w_start, cdcacm->w_end - cdcacm->w_start);

    for(i=0;i<64;i++)
        cirbuf_readbyte(cdcacm->outbuf, &ubuf[i]);
    
    usbd_ep_write_packet(cdcacm->usbd_dev, 0x82, ubuf, 64);    

    if (cirbuf_bytesinuse(cdcacm->outbuf) == 0) {
        frosted_mutex_unlock(cdcacm->dev->mutex);
        cdcacm->w_start = NULL;
        cdcacm->w_end = NULL;
        return len;
    }


    if (cdcacm->w_start < cdcacm->w_end)
    {
        cdcacm->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(cdcacm->dev->mutex);
        return SYS_CALL_AGAIN;
    }

    frosted_mutex_unlock(cdcacm->dev->mutex);
    cdcacm->w_start = NULL;
    cdcacm->w_end = NULL;
    return len;

}
static int devcdcacm_read(struct fnode *fno, void *buf, unsigned int len)
{
    int out;
    volatile int len_available;
    char *ptr = (char *)buf;
    struct dev_cdcacm *cdcacm;

    if (len <= 0)
        return len;

    cdcacm = (struct dev_cdcacm *)FNO_MOD_PRIV(fno, &mod_devcdcacm);
    if (!cdcacm)
        return -1;

    frosted_mutex_lock(cdcacm->dev->mutex);
    len_available =  cirbuf_bytesinuse(cdcacm->inbuf);
    if (len_available <= 0) {
        cdcacm->dev->pid = scheduler_get_cur_pid();
        task_suspend();
        frosted_mutex_unlock(cdcacm->dev->mutex);
        out = SYS_CALL_AGAIN;
        goto again;
    }

    if (len_available < len)
        len = len_available;

    for(out = 0; out < len; out++) {
        /* read data */
        if (cirbuf_readbyte(cdcacm->inbuf, ptr) != 0)
            break;
        ptr++;
    }

again:
    frosted_mutex_unlock(cdcacm->dev->mutex);
    return out;

}
static int devcdcacm_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
}

static int cdcacm_control_request(usbd_device * usbd_dev,
				  struct usb_setup_data *req, uint8_t ** buf,
				  uint16_t * len,
				  void (**complete) (usbd_device * usbd_dev,
						     struct usb_setup_data *
						     req))
{
	uint8_t dtr, rts;

	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:{
			/*
			 * This Linux cdc_acm driver requires this to be implemented
			 * even though it's optional in the CDC spec, and we don't
			 * advertise it in the ACM functional descriptor.
			 */

			dtr = (req->wValue & (1 << 0)) ? 1 : 0;
			rts = (req->wValue & (1 << 1)) ? 1 : 0;

			glue_set_line_state_cb(dtr, rts);

			return 1;
		}
	case USB_CDC_REQ_SET_LINE_CODING:{
			struct usb_cdc_line_coding *coding;

			if (*len < sizeof(struct usb_cdc_line_coding))
				return 0;

			coding = (struct usb_cdc_line_coding *)*buf;
			return glue_set_line_coding_cb(coding->dwDTERate,
						       coding->bDataBits,
						       coding->bParityType,
						       coding->bCharFormat);
		}
	}
	return 0;
}

static void cdcacm_data_rx_cb(usbd_device * usbd_dev, uint8_t ep)
{
        int i;
	uint8_t buf[64];

	(void)ep;

	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);
        for(i=0;i<len;i++)
            cirbuf_writebyte(DEV_CDCACM[0].inbuf, buf[i]);

        if (DEV_CDCACM[0].dev->pid > 0) 
            task_resume(DEV_CDCACM[0].dev->pid);
}

static void cdcacm_data_tx_complete_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    int ret;
    int i, len;
    uint8_t ubuf[64];

    if (DEV_CDCACM[0].tx_busy == 0)
        return;

    frosted_mutex_lock(&DEV_CDCACM[0].dev->mutex);

    len = cirbuf_bytesinuse(DEV_CDCACM[0].outbuf);
    if (len > 64)
        len = 64;

    for(i=0;i<64;i++)
        cirbuf_readbyte(DEV_CDCACM[0].outbuf, &ubuf[i]);

    frosted_mutex_unlock(&DEV_CDCACM[0].dev->mutex);
    
    usbd_ep_write_packet(&DEV_CDCACM[0].usbd_dev, 0x82, ubuf, 64);    
    
    if (cirbuf_bytesinuse(DEV_CDCACM[0].outbuf) == 0) 
        DEV_CDCACM[0].tx_busy = 0;

    if (DEV_CDCACM[0].dev->pid > 0) 
        task_resume(DEV_CDCACM[0].dev->pid);
}

static void cdcacm_set_config(usbd_device * usbd_dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_tx_complete_cb);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(usbd_dev,
				       USB_REQ_TYPE_CLASS |
				       USB_REQ_TYPE_INTERFACE,
				       USB_REQ_TYPE_TYPE |
				       USB_REQ_TYPE_RECIPIENT,
				       cdcacm_control_request);
}

static void cdcacm_fno_init(struct fnode *dev, uint32_t n, const struct cdcacm_addr * addr)
{
    struct dev_cdcacm *c = &DEV_CDCACM[n];

    char name[8] = "ttyACM";
    name[6] =  '0' + n;

    c->usb_fnode = fno_search(addr->usb_name);
    if(c->usb_fnode == NULL)
        return;
    
    c->dev = device_fno_init(&mod_devcdcacm, name, dev, FL_TTY, c);
    c->usbd_dev =  usb_register_set_config_callback(c->usb_fnode, cdcacm_set_config);
    c->inbuf = cirbuf_create(128);
    c->outbuf = cirbuf_create(128);
    c->tx_busy = 0;
}

void cdcacm_init(struct fnode *dev, const struct cdcacm_addr cdcacm_addrs[], int num_cdcacm)
{

    int i;
    for (i = 0; i < num_cdcacm; i++) 
    {
        cdcacm_fno_init(dev, i, &cdcacm_addrs[i]);
    }
    register_module(&mod_devcdcacm);
}

