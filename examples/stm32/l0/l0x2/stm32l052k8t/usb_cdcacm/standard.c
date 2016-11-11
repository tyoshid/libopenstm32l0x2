/*
 * standard.c
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

#include <usb/standard.h>
#include <usb/langids.h>

#include <usbdevfs.h>

#include "usb_cdcacm.h"

int get_descriptor(int type, int index, int langid, char **data);
bool class_request_error(struct usb_setup_data *req);
bool class_request_data_error(struct usb_setup_data *req, char *buf);
int class_request(struct usb_setup_data *req, char *buf, char **data);
void class_reset(void);

enum usb_device_state device_state = USB_STATE_POWERED;
volatile enum ep_state endpoint_state[MAXENDPOINT];

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
		    (index && (req->wIndex != LANGID_ENGLISH_US &&
			       req->wIndex != LANGID_JAPANESE)) ||
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
	len = get_descriptor(dt, index, req->wIndex, data);
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
	    device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_configuration(char *buf, char **data)
{
	if (device_state == USB_STATE_ADDRESS)
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
	if (req->wIndex >= MAXINTERFACE || device_state == USB_STATE_ADDRESS)
		return true;
	if (req->wValue || req->wLength != 1 ||
	    device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_interface(char *buf, char **data)
{
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
		    device_state != USB_STATE_CONFIGURED)
			return true;
		break;
	case (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_ENDPOINT):
		if (req->wIndex && device_state != USB_STATE_CONFIGURED)
			return true;
		if (req->wIndex && req->wIndex != NOTIFICATION_ENUM &&
		    req->wIndex != DATA_RX_ENUM && req->wIndex != DATA_TX_ENUM)
			return true;
		break;
	default:
		return true;
	}
	if (req->wValue || req->wLength != 2 ||
	    device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_get_status(struct usb_setup_data *req, char *buf,
			      char **data)
{
	short *p;

	p = (short *)buf;
	*p = 0;
	if ((req->bmRequestType & USB_RECIP_MASK) == USB_RECIP_ENDPOINT) {
		switch (req->wIndex) {
		case NOTIFICATION_ENUM:
			if (endpoint_state[NOTIFICATION_INDEX] ==
			    EP_STATE_HALT)
				*p = USB_DEV_STATUS_HALT;
			break;
		case DATA_RX_ENUM:
			if (endpoint_state[DATA_RX_INDEX] == EP_STATE_HALT)
				*p = USB_DEV_STATUS_HALT;
			break;
		case DATA_TX_ENUM:
			if (endpoint_state[DATA_TX_INDEX] == EP_STATE_HALT)
				*p = USB_DEV_STATUS_HALT;
			break;
		default:
			break;
		}
	}
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
	    device_state == USB_STATE_CONFIGURED)
		return true;
	return false;
}

static int request_set_address(struct usb_setup_data *req)
{
	if (device_state == USB_STATE_DEFAULT) {
		if (req->wValue)
			device_state = USB_STATE_ADDRESS;
	} else if (device_state == USB_STATE_ADDRESS) {
		if (!req->wValue)
			device_state = USB_STATE_DEFAULT;
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
	if (req->wIndex || req->wLength || device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_set_configuration(struct usb_setup_data *req)
{
	if (device_state == USB_STATE_ADDRESS) {
		if (req->wValue) {
			device_state = USB_STATE_CONFIGURED;
			endpoint_state[NOTIFICATION_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(NOTIFICATION_INDEX);
			endpoint_state[DATA_RX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_rx(DATA_RX_INDEX);
			endpoint_state[DATA_TX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(DATA_TX_INDEX);
		}
	} else if (device_state == USB_STATE_CONFIGURED) {
		if (!req->wValue) {
			device_state = USB_STATE_ADDRESS;
			endpoint_state[NOTIFICATION_INDEX] = EP_STATE_DISABLE;
			usbdevfs_disable_ep_tx(NOTIFICATION_INDEX);
			endpoint_state[DATA_RX_INDEX] = EP_STATE_DISABLE;
			usbdevfs_disable_ep_rx(DATA_RX_INDEX);
			endpoint_state[DATA_TX_INDEX] = EP_STATE_DISABLE;
			usbdevfs_disable_ep_tx(DATA_TX_INDEX);
		}
	}
	if (endpoint_state[NOTIFICATION_INDEX] == EP_STATE_HALT) {
		endpoint_state[NOTIFICATION_INDEX] = EP_STATE_ENABLE;
		usbdevfs_enable_ep_tx(NOTIFICATION_INDEX);
	}
	if (endpoint_state[DATA_RX_INDEX] == EP_STATE_HALT) {
		endpoint_state[DATA_RX_INDEX] = EP_STATE_ENABLE;
		usbdevfs_enable_ep_rx(DATA_RX_INDEX);
	}
	if (endpoint_state[DATA_TX_INDEX] == EP_STATE_HALT) {
		endpoint_state[DATA_TX_INDEX] = EP_STATE_ENABLE;
		usbdevfs_enable_ep_tx(DATA_TX_INDEX);
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
	if (req->wValue || req->wIndex >= MAXINTERFACE ||
	    device_state == USB_STATE_ADDRESS)
		return true;
	if (req->wLength || device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_set_interface(struct usb_setup_data *req)
{
	if (req->wIndex == INTERFACE_COMM) {
		if (endpoint_state[NOTIFICATION_INDEX] == EP_STATE_HALT) {
			endpoint_state[NOTIFICATION_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(NOTIFICATION_INDEX);
		}
	} else if (req->wIndex == INTERFACE_DATA) {
		if (endpoint_state[DATA_RX_INDEX] == EP_STATE_HALT) {
			endpoint_state[DATA_RX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_rx(DATA_RX_INDEX);
		}
		if (endpoint_state[DATA_TX_INDEX] == EP_STATE_HALT) {
			endpoint_state[DATA_TX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(DATA_TX_INDEX);
		}
	}
	return 0;
}

/*
 * Set Feature
 *
 * | bmRequestType | bRequest | wValue   | wIndex    | wLength
 * | 00000000B     | 3        | Feature  | Zero      | Zero
 * | 00000001B     |          | Selector | Interface |
 * | 00000010B     |          |          | Endpoint  |
 *
 * Defaut state:	Not specified.
 * Address state:	This request is valid.
 * Configured state:	This request is valid.
 */
static bool set_feature_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_STANDARD |
				   USB_RECIP_ENDPOINT))
		return true;
	if (req->wValue != USB_FEAT_ENDPOINT_HALT)
		return true;
	if (req->wIndex && device_state != USB_STATE_CONFIGURED)
		return true;
	if (req->wIndex && req->wIndex != NOTIFICATION_ENUM &&
	    req->wIndex != DATA_RX_ENUM &&
	    req->wIndex != DATA_TX_ENUM)
		return true;
	if (req->wLength || device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}
	
static int request_set_feature(struct usb_setup_data *req)
{
	switch (req->wIndex) {
	case NOTIFICATION_ENUM:
		endpoint_state[NOTIFICATION_INDEX] = EP_STATE_HALT;
		usbdevfs_halt_ep_tx(NOTIFICATION_INDEX);
		break;
	case DATA_RX_ENUM:
		endpoint_state[DATA_RX_INDEX] = EP_STATE_HALT;
		usbdevfs_halt_ep_rx(DATA_RX_INDEX);
		break;
	case DATA_TX_ENUM:
		endpoint_state[DATA_TX_INDEX] = EP_STATE_HALT;
		usbdevfs_halt_ep_tx(DATA_TX_INDEX);
		break;
	default:
		break;
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
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_STANDARD |
				   USB_RECIP_ENDPOINT))
		return true;
	if (req->wValue != USB_FEAT_ENDPOINT_HALT)
		return true;
	if (req->wIndex && device_state != USB_STATE_CONFIGURED)
		return true;
	if (req->wIndex && req->wIndex != NOTIFICATION_ENUM &&
	    req->wIndex != DATA_RX_ENUM &&
	    req->wIndex != DATA_TX_ENUM)
		return true;
	if (req->wLength || device_state == USB_STATE_DEFAULT)
		return true;
	return false;
}

static int request_clear_feature(struct usb_setup_data *req)
{
	switch (req->wIndex) {
	case NOTIFICATION_ENUM:
		if (endpoint_state[NOTIFICATION_INDEX] == EP_STATE_HALT) {
			endpoint_state[NOTIFICATION_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(NOTIFICATION_INDEX);
			usbdevfs_reset_ep_data_toggle(NOTIFICATION_INDEX);
		}
		break;
	case DATA_RX_ENUM:
		if (endpoint_state[DATA_RX_INDEX] == EP_STATE_HALT) {
			endpoint_state[DATA_RX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_rx(DATA_RX_INDEX);
			usbdevfs_reset_ep_data_toggle(DATA_RX_INDEX);
		}
		break;
	case DATA_TX_ENUM:
		if (endpoint_state[DATA_TX_INDEX] == EP_STATE_HALT) {
			endpoint_state[DATA_TX_INDEX] = EP_STATE_ENABLE;
			usbdevfs_enable_ep_tx(DATA_TX_INDEX);
			usbdevfs_reset_ep_data_toggle(DATA_TX_INDEX);
		}
		break;
	default:
		break;
	}
	return 0;
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
	case USB_REQ_SET_FEATURE:
		return set_feature_error(req);
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
		return request_get_interface(buf, data);
	case USB_REQ_GET_STATUS:
		return request_get_status(req, buf, data);
	case USB_REQ_SET_ADDRESS:
		return request_set_address(req);
	case USB_REQ_SET_CONFIGURATION:
		return request_set_configuration(req);
	case USB_REQ_SET_INTERFACE:
		return request_set_interface(req);
	case USB_REQ_SET_FEATURE:
		return request_set_feature(req);
	case USB_REQ_CLEAR_FEATURE:
		return request_clear_feature(req);
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
	} else if ((req->bmRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS &&
		   device_state == USB_STATE_CONFIGURED) {
		if (stage == CONTROL_STAGE_SETUP) {
			if (class_request_error(req))
				return -1;
			if (req->bmRequestType & USB_DIR_IN)
				return class_request(req, buf, data);
		} else if (stage == CONTROL_STAGE_DATA) {
			if (class_request_data_error(req, buf))
				return -1;
		} else if (stage == CONTROL_STAGE_STATUS) {
			if (!(req->bmRequestType & USB_DIR_IN))
				return class_request(req, buf, 0);
		}
	} else {
		return -1;
	}
	return 0;
}

void standard_reset(void)
{
	class_reset();
	device_state = USB_STATE_DEFAULT;
	endpoint_state[NOTIFICATION_INDEX] = EP_STATE_DISABLE;
	endpoint_state[DATA_RX_INDEX] = EP_STATE_DISABLE;
	endpoint_state[DATA_TX_INDEX] = EP_STATE_DISABLE;
}
