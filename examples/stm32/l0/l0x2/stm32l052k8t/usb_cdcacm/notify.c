/*
 * notify.c
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
#include <usb/cdc.h>

#include <usart.h>

#include "usb_cdcacm.h"

static volatile bool response_available;
alignas(2) static const struct usb_cdc_notification notification_response = {
	.bmRequestType = (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE),
	.bNotification = USB_CDC_NOTIFY_RESPONSE_AVAILABLE,
	.wValue = 0,
	.wIndex = INTERFACE_COMM,
	.wLength = 0
};
static volatile short serial_state;
struct notif_state {
	struct usb_cdc_notification header;
	short data;
} __attribute__ ((packed));
alignas(2) static struct notif_state notification_state = {
	.header = {.bmRequestType = (USB_DIR_IN | USB_TYPE_CLASS |
				     USB_RECIP_INTERFACE),
		   .bNotification = USB_CDC_NOTIFY_SERIAL_STATE,
		   .wValue = 0,
		   .wIndex = INTERFACE_COMM,
		   .wLength = 2
	},
};

void notify_response_available(void)
{
	response_available = true;
}

void notify_serial_state(int usart_status)
{
	if (usart_status & USART_ORE)
		serial_state |= USB_CDC_SERIAL_STATE_OVERRUN;
	if (usart_status & USART_FE)
		serial_state |= USB_CDC_SERIAL_STATE_FRAMING;
	if (usart_status & USART_PE)
		serial_state |= USB_CDC_SERIAL_STATE_PARITY;
}

bool notify_available(void)
{
	return response_available || serial_state;
}

int notify(char **data)
{
	if (response_available) {
		response_available = false;
		*data = (char *)&notification_response;
		return sizeof(notification_response);
	}
	if (serial_state) {
		notification_state.data = serial_state;
		serial_state = 0;
		*data = (char *)&notification_state;
		return sizeof(notification_state);
	}
	return 0;
}

void notify_reset(void)
{
	response_available = false;
	serial_state = 0;
}
