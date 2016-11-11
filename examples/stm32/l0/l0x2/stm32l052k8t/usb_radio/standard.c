/*
 * standard.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_radio.
 *
 * usb_radio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_radio.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>

#include <usb/standard.h>
#include <usb/langids.h>

#include <usbdevfs.h>

#include "usb_radio.h"

int get_descriptor(int type, int index, char **data);

static enum usb_device_state state = USB_STATE_POWERED;
enum ep_state endpoint_state[MAXENDPOINT];

/*
 * Get Descriptor
 *
 * | bmRequestType | bRequest | wValue | wIndex      | wLength
 * | 10000000B     | 6        | Type & | Zero or     | Length
 * |               |          | Index  | Language ID |
 *
 * Defaut state:	This request is valid.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool get_descriptor_error(struct usb_setup_data *req)
{
	int dt;
	int index;

	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				   USB_RECIP_DEVICE))
		return true;
	dt = req->wValue >> 8;
	index = req->wValue & 0xff;
	switch (dt) {
	case USB_DT_DEVICE:
		if (index || req->wIndex)
			return true;
		break;
	case USB_DT_CONFIGURATION:
		if (index || req->wIndex)
			return true;
		break;
	case USB_DT_STRING:
		if ((!index && req->wIndex) ||
		    (index && req->wIndex != LANGID_ENGLISH_US) ||
		    (index > MAX_STRING_INDEX))
			return true;
		break;
	default:
		return true;

	}
	return false;
}

static int request_get_descriptor(struct usb_setup_data *req, char **data)
{
	int dt;
	int index;
	int len;

	dt = req->wValue >> 8;
	index = req->wValue & 0xff;
	len = get_descriptor(dt, index, data);
	if (len > req->wLength)
		len = req->wLength;
	return len;
}

/*
 * Get Configuration
 *
 * | bmRequestType | bRequest | wValue | wIndex | wLength
 * | 10000000B     | 8        | Zero   | Zero   | One
 *
 * Defaut state:	Not specified.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool get_configuration_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				   USB_RECIP_DEVICE))
		return true;
	if (req->wValue || req->wIndex || req->wLength != 1 ||
	    state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_configuration(char *buf, char **data)
{
	if (state == USB_STATE_ADDRESS)
		*buf = 0;
	else
		*buf = CONFIGURATION_VALUE;
	*data = buf;
	return 1;
}

/*
 * Get Interface
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength
 * | 10000001B     | 10       | Zero   | Interface | One
 *
 * Defaut state:	Not specified.
 * Address state:	Request Error.
 * Configured state:	This request is valid.
 */
static bool get_interface_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex >= MAXINTERFACE || state == USB_STATE_ADDRESS)
		return true;
	if (req->wValue || req->wLength != 1 ||
	    state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_interface(struct usb_setup_data *req, char *buf,
				 char **data)
{
	if (req->wIndex == INTERFACE_AS &&
	    endpoint_state[1] == EP_STATE_ENABLE)
		*buf = 1;
	else
		*buf = 0;
	*data = buf;
	return 1;
}

/*
 * Get Status
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength
 * | 10000000B     | 0        | Zero   | Zero      | Two
 * | 10000001B     |          |        | Interface |
 * | 10000010B     |          |        | Endpoint  |
 *
 * Defaut state:	Not specified.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool get_status_error(struct usb_setup_data *req)
{
	switch (req->bmRequestType) {
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE):
		if (req->wIndex)
			return true;
		break;
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE):
		if (req->wIndex >= MAXINTERFACE ||
		    state != USB_STATE_CONFIGURED)
			return true;
		break;
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT):
		if (req->wIndex && state != USB_STATE_CONFIGURED)
			return true;
		if (req->wIndex &&
		    req->wIndex != AS_ENUM)
			return true;
		break;
	default:
		return true;
	}
	if (req->wValue || req->wLength != 2 || state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_status(struct usb_setup_data *req, char *buf,
			      char **data)
{
	short *p;

	p = (short *)buf;
	if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_DEVICE)
		*p = USB_DEV_STATUS_SELF_POWERED;
	else
		*p = 0;
	*data = buf;
	return sizeof(short);
}

/*
 * Set Address
 *
 * | bmRequestType | bRequest | wValue  | wIndex | wLength
 * | 00000000B     | 5        | Address | Zero   | Zero
 *
 * Defaut state:	This request is valid.
 * Address state:	This request is valid.
 * Configured state:	Not specified.
 */
static bool set_address_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_STANDARD |
				   USB_RECIP_DEVICE))
		return true;
	if (req->wValue > 127 || req->wIndex || req->wLength ||
	    state == USB_STATE_CONFIGURED)
		return true;
	return false;
}

static int request_set_address(struct usb_setup_data *req)
{
	if (state == USB_STATE_DEFAULT) {
		if (req->wValue)
			state = USB_STATE_ADDRESS;
	} else if (state == USB_STATE_ADDRESS) {
		if (!req->wValue)
			state = USB_STATE_DEFAULT;
	}
	usbdevfs_set_device_address(req->wValue);
	return 0;
}

/*
 * Set Configuration
 *
 * | bmRequestType | bRequest | wValue  | wIndex | wLength
 * | 00000000B     | 9        | Value   | Zero   | Zero
 *
 * Defaut state:	Not specified.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool set_configuration_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_STANDARD |
				   USB_RECIP_DEVICE))
		return true;
	if (req->wValue && req->wValue != CONFIGURATION_VALUE)
		return true;
	if (req->wIndex || req->wLength || state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_set_configuration(struct usb_setup_data *req)
{
	if (state == USB_STATE_ADDRESS) {
		if (req->wValue)
			state = USB_STATE_CONFIGURED;
	} else if (state == USB_STATE_CONFIGURED) {
		if (!req->wValue)
			state = USB_STATE_ADDRESS;
	}
	if (endpoint_state[1] == EP_STATE_ENABLE) {
		usbdevfs_disable_ep_tx(1);
		endpoint_state[1] = EP_STATE_DISABLE;
	}
	return 0;
}

/*
 * Set Interface
 *
 * | bmRequestType | bRequest | wValue    | wIndex    | wLength
 * | 00000001B     | 11       | Alternate | Interface | Zero
 * |               |          | Setting   |           |
 *
 * Defaut state:	Not specified.
 * Address state:	Request Error.
 * Configured state:	This request is valid.
 */
static bool set_interface_error(struct usb_setup_data *req)
{
	if (req->bmRequestType !=
	    (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE))
		return true;
	if (state == USB_STATE_ADDRESS)
		return true;
	switch (req->wIndex) {
	case INTERFACE_AC:
		if (req->wValue)
			return true;
		break;
	case INTERFACE_AS:
		if (req->wValue > 1)
			return true;
		break;
	default:
		return true;
	}
	if (req->wLength || state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_set_interface(struct usb_setup_data *req)
{
	if (req->wValue) {
		if (endpoint_state[1] == EP_STATE_DISABLE) {
			usbdevfs_enable_ep_tx(1);
			endpoint_state[1] = EP_STATE_ENABLE;
		}
	} else {
		if (req->wIndex == INTERFACE_AS) {
			if (endpoint_state[1] == EP_STATE_ENABLE) {
				usbdevfs_disable_ep_tx(1);
				endpoint_state[1] = EP_STATE_DISABLE;
			}
		}
	}
	return 0;
}

/*
 * Clear Feature
 *
 * | bmRequestType | bRequest | wValue   | wIndex    | wLength
 * | 00000000B     | 1        | Feature  | Zero      | Zero
 * | 00000001B     |          | Selector | Interface |
 * | 00000010B     |          |          | Endpoint  |
 *
 * Default state:	Not specified.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool clear_feature_error(struct usb_setup_data *req)
{
	switch (req->bmRequestType) {
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE):
		if (req->wIndex)
			return true;
		break;
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE):
		if (req->wIndex >= MAXINTERFACE ||
		    state != USB_STATE_CONFIGURED)
			return true;
		break;
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT):
		if (req->wIndex && state != USB_STATE_CONFIGURED)
			return true;
		if (req->wIndex && req->wIndex != AS_ENUM)
			return true;
		break;
	default:
		return true;
	}
	if (req->wLength || state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static bool standard_request_error(struct usb_setup_data *req)
{
	switch (req->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		return get_descriptor_error(req);
	case USB_REQ_GET_CONFIGURATION:
		return get_configuration_error(req);
	case USB_REQ_GET_INTERFACE:
		return get_interface_error(req);
	case USB_REQ_GET_STATUS:
		return get_status_error(req);
	case USB_REQ_SET_ADDRESS:
		return set_address_error(req);
	case USB_REQ_SET_CONFIGURATION:
		return set_configuration_error(req);
	case USB_REQ_SET_INTERFACE:
		return set_interface_error(req);
	case USB_REQ_CLEAR_FEATURE:
		return clear_feature_error(req);
	default:
		break;
	}
	return true;
}

static int standard_request(struct usb_setup_data *req, char *buf, char **data)
{
	switch (req->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		return request_get_descriptor(req, data);
	case USB_REQ_GET_CONFIGURATION:
		return request_get_configuration(buf, data);
	case USB_REQ_GET_INTERFACE:
		return request_get_interface(req, buf, data);
	case USB_REQ_GET_STATUS:
		return request_get_status(req, buf, data);
	case USB_REQ_SET_ADDRESS:
		return request_set_address(req);
	case USB_REQ_SET_CONFIGURATION:
		return request_set_configuration(req);
	case USB_REQ_SET_INTERFACE:
		return request_set_interface(req);
	case USB_REQ_CLEAR_FEATURE:
		return 0;	/* nothing to do */
	default:
		break;
	}
	return -1;
}

int usb_device_request(struct usb_setup_data *req, enum control_stage stage,
		       char *buf, char **data)
{
	if ((req->bmRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		if (stage == CONTROL_STAGE_SETUP) {
			if (standard_request_error(req))
				return -1;
			if (req->bmRequestType & USB_DIR_IN)
				return standard_request(req, buf, data);
		} else if (stage == CONTROL_STAGE_STATUS) {
			if (!(req->bmRequestType & USB_DIR_IN))
				return standard_request(req, 0, 0);
		}
	} else {
		return -1;
	}
	return 0;
}

void standard_reset(void)
{
	state = USB_STATE_DEFAULT;
	endpoint_state[0] = EP_STATE_ENABLE;
	endpoint_state[1] = EP_STATE_DISABLE;
}
