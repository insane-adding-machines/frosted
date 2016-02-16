#ifndef STM32F4_USBDEF
#define STM32F4_USBDEF 

#ifdef CONFIG_STM32F4USB

#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/usbd.h>
#include "stm32f4_usb.h"
#include <libopencm3/usb/cdc.h>

#ifdef CONFIG_DEVUSBCDCACM
#include "cdc_acm.h"
#endif

#ifdef CONFIG_DEVUSBCDCECM
#include "cdc_ecm.h"
#endif



#ifdef CONFIG_DEVUSBCDCACM
static const struct usb_endpoint_descriptor cdc_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 1,
}};

static const struct usb_endpoint_descriptor cdc_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__ ((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = (1 << 1),
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	}
};

static const struct usb_interface_descriptor cdc_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = cdc_comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors)
}};

static const struct usb_interface_descriptor cdc_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = cdc_data_endp,
}};

#endif

#ifdef CONFIG_DEVUSBCDCECM
static const struct usb_endpoint_descriptor eth2_comm_endp[] = {
    {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x82,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 16,
    .bInterval = 0x10,
    }, 
};

static const struct usb_endpoint_descriptor eth2_data_endp[] = {
    {
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
    } 
};

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

static const struct usb_interface_descriptor eth2_comm_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = USB_CLASS_CDC,
    .bInterfaceSubClass = USB_CDC_SUBCLASS_ECM,
    .bInterfaceProtocol = USB_CDC_PROTOCOL_NONE,
    .iInterface = 0,

    .endpoint = eth2_comm_endp,

    .extra = &cdcecm_functional_descriptors,
    .extralen = sizeof(cdcecm_functional_descriptors)
} };

static const struct usb_interface_descriptor eth2_data_iface[] = {{
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = USB_DT_INTERFACE,
    .bInterfaceNumber = 1,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = USB_CLASS_DATA,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0,
    .endpoint = eth2_data_endp,
} };
#endif

static const struct usb_interface ifaces[] = {
#ifdef CONFIG_DEVUSBCDCECM
    {
        .num_altsetting = 1,
        .altsetting = eth2_comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting = eth2_data_iface,
    } 
#endif
#ifdef CONFIG_DEVUSBCDCACM
    {
        .num_altsetting = 1,
        .altsetting = cdc_comm_iface,
    },
    {
        .num_altsetting = 1,
        .altsetting = cdc_data_iface,
    } 
#endif
};

#define NUM_USB_INTERFACES (sizeof(ifaces)/sizeof(struct usb_interface))

static const struct usb_config_descriptor config[] = {
    {
    .bLength = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType = USB_DT_CONFIGURATION,
    .wTotalLength = 71,
    .bNumInterfaces = NUM_USB_INTERFACES,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .bMaxPower = 0x32,
    .interface = ifaces,
    }
};

#define NUM_USB_CONFIGS (sizeof(config)/sizeof(struct usb_config_descriptor))

static const struct usb_device_descriptor usbdev_desc= {
    .bLength = USB_DT_DEVICE_SIZE,
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0,                          /* Set the Dev Class to 0 - interface(s) specify class */
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = 0x0200,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = NUM_USB_CONFIGS,
};

static const char usb_string_manuf[] = "Insane adding machines";
static const char usb_string_name[] = "Frosted Eth gadget";
static const char usb_serialn[] = "01";
static const char usb_macaddr[] = "005af341b4c9";


static const char *usb_strings[] = {
    usb_string_manuf, usb_string_name, usb_serialn, usb_macaddr
};

#define NUM_USB_STRINGS (sizeof(usb_strings)/sizeof(char *))

static const struct usb_addr usb_addrs[] = {
    {
    .irq = NVIC_OTG_FS_IRQ,
    .rcc = RCC_OTGFS,
    .name = "usb",
    .num_callbacks = 1,
    .usbdev_desc= &usbdev_desc,
    .config = config,
    .usb_strings = usb_strings,
    .num_usb_strings = NUM_USB_STRINGS,
    },
};
#define NUM_USB (sizeof(usb_addrs)/sizeof(struct usb_addr))


#ifdef CONFIG_DEVUSBCDCACM
static const struct cdcacm_addr cdcacm_addrs[] = {
    {
        .name = "ttyACMx",
        .usb_name = "/dev/usb",   
    },
};
#define NUM_USBCDCACM (sizeof(cdcacm_addrs)/sizeof(struct cdcacm_addr))

#endif

#endif

#endif
