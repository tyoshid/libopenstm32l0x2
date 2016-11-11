/*
 * control.c
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

#include "usb_custom.h"

int usb_device_request(struct usb_setup_data *req);
void standard_reset(void);

static enum usbdevfs_control_state state; /* Control transfer state */
static struct usb_setup_data curreq;	  /* Current device request */

/* SETUP transaction */
void control_setup(char *buf, int len)
{
	struct usb_setup_data *req;

	if (len != sizeof(struct usb_setup_data)) {
		state = USBDEVFS_STALL;
		usbdevfs_set_control_state(0, state);
		return;
	}
	req = (struct usb_setup_data *)buf;
	if (req->bmRequestType & USB_DIR_IN) {
		if (usb_device_request(req)) {
			state = USBDEVFS_STALL;
			usbdevfs_set_control_state(0, state);
			return;
		}
		state = USBDEVFS_LAST_DATA_IN;
		usbdevfs_set_control_state(0, state);
	} else {
		curreq = *req;
		usbdevfs_write(0, 0, 0);
		state = USBDEVFS_STATUS_IN;
		usbdevfs_set_control_state(0, state);
	}
}

/* OUT transaction */
void control_rx(void)
{
	state = USBDEVFS_STALL;
	usbdevfs_set_control_state(0, state);
}

/* IN transaction */
void control_tx(void)
{
	switch (state) {
	case USBDEVFS_LAST_DATA_IN:
		state = USBDEVFS_STATUS_OUT;
		break;
	case USBDEVFS_STATUS_IN:
		usb_device_request(&curreq);
		state = USBDEVFS_STALL;
		break;
	default:
		state = USBDEVFS_STALL;
		break;
	}
	usbdevfs_set_control_state(0, state);
}

void control_reset(void)
{
	standard_reset();
	state = USBDEVFS_STALL;
	usbdevfs_set_control_state(0, state);
}
