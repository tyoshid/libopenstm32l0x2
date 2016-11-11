/*
 * standard.c
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

#include <usb/standard.h>

#include <usbdevfs.h>

int vendor_request(struct usb_setup_data *req);
int get_descriptor(int type, char **data);

static enum usb_device_state state = USB_STATE_POWERED;

static int standard_request(struct usb_setup_data *req)
{
	int dt;
	int n;
	int len;
	char *desc;
	
	switch (req->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		dt = (req->wValue >> 8);
		n = req->wLength;
		len = get_descriptor(dt, &desc);
		if (len == 0)
			return -1;
		if (len < req->wLength)
			n = len;
		usbdevfs_write(0, desc, n);
		break;
	case USB_REQ_SET_ADDRESS:
		usbdevfs_set_device_address(req->wValue);
		if (state == USB_STATE_DEFAULT) {
			if (req->wValue)
				state = USB_STATE_ADDRESS;
		} else if (state == USB_STATE_ADDRESS) {
			if (!req->wValue)
				state = USB_STATE_DEFAULT;
		}
		break;
	case USB_REQ_SET_CONFIGURATION:
		if (state == USB_STATE_ADDRESS) {
			if (req->wValue == 1)
				state = USB_STATE_CONFIGURED;
		} else if (state == USB_STATE_CONFIGURED) {
			if (!req->wValue)
				state = USB_STATE_ADDRESS;
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int usb_device_request(struct usb_setup_data *req)
{
	if ((req->bmRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD)
		return standard_request(req);
	else if ((req->bmRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR &&
		 state == USB_STATE_CONFIGURED)
		return vendor_request(req);
	return -1;
}

void standard_reset(void)
{
	state = USB_STATE_DEFAULT;
}
