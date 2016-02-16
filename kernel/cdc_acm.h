#ifndef INC_CDC_ACM_H
#define INC_CDC_ACM_H


struct cdcacm_addr {
    const char * name;
    const char * usb_name;
};

void cdcacm_init(struct fnode *dev, const struct cdcacm_addr cdcacm_addrs[], int num_cdcacm);


#endif

