/*
 * descriptor.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_cdcacm.
 *
 * usb_cdcacm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_cdcacm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_cdcacm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#include <usb/standard.h>
#include <usb/langids.h>
#include <usb/cdc.h>

#include "usb_cdcacm.h"

/* struct usb_cdc_union_descriptor_1 */
USB_CDC_UNION_DESCRIPTOR(1);
/* Communications Class Specific Interface Descriptor */
struct cdcacm_functional_descriptors {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor_1 cdc_union;
	struct usb_cdc_call_management_descriptor call_management;
} __attribute__ ((packed));
/* Configuration */
struct config_descriptor {
	struct usb_config_descriptor config;
	struct usb_interface_descriptor comm_if;
	struct cdcacm_functional_descriptors cdcacm_func;
	struct usb_endpoint_descriptor comm_endp;
	struct usb_interface_descriptor data_if;
	struct usb_endpoint_descriptor rx_endp;
	struct usb_endpoint_descriptor tx_endp;
} __attribute__ ((packed));
/* struct usb_string_descriptor_2 */
USB_STRING_DESCRIPTOR(2);
/* struct usb_string_descriptor_17 */
USB_STRING_DESCRIPTOR(17);
/* struct usb_string_descriptor_20 */
USB_STRING_DESCRIPTOR(20);
/* struct usb_string_descriptor_14 */
USB_STRING_DESCRIPTOR(14);
/* struct usb_string_descriptor_29 */
USB_STRING_DESCRIPTOR(29);

/* Device Descriptor */
alignas(2) static const struct usb_device_descriptor dev_desc = {
	.bLength = sizeof(struct usb_device_descriptor),
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_COMM,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = MAXPACKETSIZE0,
	.idVendor = 0x2975,
	.idProduct = 0x0003,
	.bcdDevice = 0x0010,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1
};

alignas(2) static const struct config_descriptor config_desc = {
	/* Configuration Descriptor */
	.config = {
		.bLength = sizeof(struct usb_config_descriptor),
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(struct config_descriptor),
		.bNumInterfaces = MAXINTERFACE,
		.bConfigurationValue = CONFIGURATION_VALUE,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATTR_D7,
		.bMaxPower = 100 / 2
	},
	/* Commucations Class Interface Descriptor */
	.comm_if = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = INTERFACE_COMM,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_COMM,
		.bInterfaceSubClass = USB_COMM_SUBCLASS_ACM,
		.bInterfaceProtocol = USB_COMM_PROTO_AT_V25TER,
		.iInterface = 0
	},
	/* Communications Class Specific Interface Descriptor */
	.cdcacm_func = {
		/* Header Functional Descriptor */
		.header = {
			.bFunctionLength =
			sizeof(struct usb_cdc_header_descriptor),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_COMM_TYPE_HEADER,
			.bcdCDC = 0x0120
		},
		/* Abstract Control Management Functional Descriptor */
		.acm = {
			.bFunctionLength =
			sizeof(struct usb_cdc_acm_descriptor),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_COMM_TYPE_ACM,
			.bmCapabilities = USB_CDC_ACM_CAP_LINE |
			USB_CDC_ACM_CAP_BREAK
		},
		/* Union Interface Functional Descriptor */
		.cdc_union = {
			.bFunctionLength =
			sizeof(struct usb_cdc_union_descriptor_1),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_COMM_TYPE_UNION,
			.bControlInterface = INTERFACE_COMM,
			.bSubordinateInterface = {INTERFACE_DATA}
		},
		/* Call Management Functional Descriptor */
		.call_management = {
			.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_COMM_TYPE_CALL_MANAGEMENT,
			.bmCapabilities = USB_CDC_CALL_MGMT_CAP_CALL_MGMT |
			USB_CDC_CALL_MGMT_CAP_DATA_INTF,
			.bDataInterface = INTERFACE_DATA
		}
	},
	/* Notification endpoint */
	.comm_endp = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = NOTIFICATION_ENUM,
		.bmAttributes = USB_ENDPOINT_TRANS_INTERRUPT,
		.wMaxPacketSize = NOTIFICATION_SIZE,
		.bInterval = NOTIFICATION_INTERVAL
	},
	/* Data Class Interface Descriptor */
	.data_if = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = INTERFACE_DATA,
		.bAlternateSetting = 0,
		.bNumEndpoints = 2,
		.bInterfaceClass = USB_CLASS_DATA,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	},
	/* Data rx endpoint */
	.rx_endp = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = DATA_RX_ENUM,
		.bmAttributes = USB_ENDPOINT_TRANS_BULK,
		.wMaxPacketSize = DATA_SIZE,
		.bInterval = DATA_INTERVAL
	},
	/* Data tx endpoint */
	.tx_endp = {
		.bLength = sizeof(struct usb_endpoint_descriptor),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = DATA_TX_ENUM,
		.bmAttributes = USB_ENDPOINT_TRANS_BULK,
		.wMaxPacketSize = DATA_SIZE,
		.bInterval = DATA_INTERVAL
	}
};

/* Strings */
alignas(2) static const struct usb_string_descriptor_2 string_desc0 = {
	.bLength = 6,
	.bDescriptorType = USB_DT_STRING,
	.wData = {LANGID_ENGLISH_US, LANGID_JAPANESE}
};
alignas(2) static const struct usb_string_descriptor_17 string_desc1 = {
	.bLength = 36,
	.bDescriptorType = USB_DT_STRING,
	.wData = u"MPC Research Ltd."
};
alignas(2) static const struct usb_string_descriptor_20 string_desc2 = {
	.bLength = 42,
	.bDescriptorType = USB_DT_STRING,
	.wData = u"STM32L052K8T CDC-ACM"
};
alignas(2) static const struct usb_string_descriptor_14 string_desc3 = {
	.bLength = 30,
	.bDescriptorType = USB_DT_STRING,
	.wData = u"有限会社エムピーシーリサーチ"
};
alignas(2) static struct usb_string_descriptor_29 string_desc4 = {
	.bLength = 60,
	.bDescriptorType = USB_DT_STRING
};

/*
 * Device ID(HEX) - Revision ID(HEX) - Lot number(ASCII) -
 * Wafer number(0-255) - unique ID(HEX)
 */
void set_serial_number(int mcu, int *unique)
{
	int d;
	int i;
	int w;
	int h;
	
	for (i = 0; i < 3; i++) {
		h = mcu >> (8 - i * 4) & 0xf;
		string_desc4.wData[i] = (h < 10) ? (h + '0') : (h - 10 + 'a');
	}
	string_desc4.wData[3] = '-';
	for (i = 0; i < 4; i++) {
		h = mcu >> (28 - i * 4) & 0xf;
		string_desc4.wData[4 + i] = (h < 10) ? (h + '0') :
			(h - 10 + 'a');
	}
	string_desc4.wData[8] = '-';

	d = *unique;
	for (i = 0; i < 3; i++)
		string_desc4.wData[9 + i] = (d >> (2 - i) * 8) & 0xff;
	string_desc4.wData[16] = '-';
	w = (d >> 24) & 0xff;
	for (i = 0; i < 3; i++) {
		if (w < 100)
			break;
		w -= 100;
	}
	string_desc4.wData[17] = i + '0';
	for (i = 0; i < 10; i++) {
		if (w < 10)
			break;
		w -= 10;
	}
	string_desc4.wData[18] = i + '0';
	string_desc4.wData[19] = w + '0';
	string_desc4.wData[20] = '-';

	d = *(unique + 1);
	for (i = 0; i < 4; i++)
		string_desc4.wData[12 + i] = (d >> (3 - i) * 8) & 0xff;

	d = *(unique + 2);
	for (i = 0; i < 8; i++) {
		h = d >> (28 - i * 4) & 0xf;
		string_desc4.wData[21 + i] = (h < 10) ? (h + '0') :
			(h - 10 + 'a');
	}
}

int get_descriptor(int type, int index, int langid, char **data)
{
	int len;

	switch (type) {
	case USB_DT_DEVICE:
		len = sizeof(dev_desc);
		*data = (char *)&dev_desc;
		break;
	case USB_DT_CONFIGURATION:
		len = sizeof(config_desc);
		*data = (char *)&config_desc;
		break;
	case USB_DT_STRING:
		switch (index) {
		case 0:
			len = sizeof(string_desc0);
			*data = (char *)&string_desc0;
			break;
		case 1:
			if (langid == LANGID_JAPANESE) {
				len = sizeof(string_desc3);
				*data = (char *)&string_desc3;
			} else {
				len = sizeof(string_desc1);
				*data = (char *)&string_desc1;
			}
			break;
		case 2:
			len = sizeof(string_desc2);
			*data = (char *)&string_desc2;
			break;
		case 3:
			len = sizeof(string_desc4);
			*data = (char *)&string_desc4;
			break;
		default:
			len = 0;
			break;
		}
		break;
	default:
		len = 0;
		break;
	}

	return len;
}
