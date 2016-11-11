/*
 * descriptor.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_custom.
 *
 * usb_custom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_custom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_custom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdalign.h>

#include <usb/standard.h>

#include "usb_custom.h"

/* Device descriptor */
alignas(2) static const struct usb_device_descriptor dev_desc = {
	.bLength = sizeof(struct usb_device_descriptor),
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_VENDOR,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = MAXPACKETSIZE0,
	.idVendor = 0x2975,
	.idProduct = 0x0001,
	.bcdDevice = 0x0010,
	.iManufacturer = 0,
	.iProduct = 0,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};

/* Configuration descriptor and interface descriptor */
struct config_descriptor {
	struct usb_config_descriptor config;
	struct usb_interface_descriptor interface;
} __attribute__ ((packed));

alignas(2) static const struct config_descriptor config_desc = {
	.config = {
		.bLength = sizeof(struct usb_config_descriptor),
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(struct config_descriptor),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATTR_D7 |
			USB_CONFIG_ATTR_SELF_POWERED,
		.bMaxPower = 0
	},
	.interface = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = 0,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 0,
		.bInterfaceSubClass = 0,
		.bInterfaceProtocol = 0,
		.iInterface = 0
	}
};

int get_descriptor(int type, char **data)
{
	int len;

	if (type == USB_DT_DEVICE) {
		len = sizeof(dev_desc);
		*data = (char *)&dev_desc;
	} else if (type == USB_DT_CONFIGURATION) {
		len = sizeof(config_desc);
		*data = (char *)&config_desc;
	} else {
		len = 0;
	}
	return len;
}
