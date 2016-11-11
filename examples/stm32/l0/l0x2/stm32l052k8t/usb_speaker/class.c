/*
 * class.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_speaker.
 *
 * usb_speaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_speaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_speaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>

#include <usb/standard.h>
#include <usb/audio10.h>

#include "usb_speaker.h"

static bool mute;
static const char mute_data[] = {-MUTE_VOLUME >> 8,
				 (-MUTE_VOLUME >> 8) | 0x40};
static short volume[3];

/*
 * Set Feature Unit Control Request
 *
 * | bmRequestType | bRequest | wValue    | wIndex          | wLength
 * | 00100001B     | SET_CUR  | CS and CN | Feature Unit ID | Length of
 * |               | SET_MIN  |           | and             | parameter
 * |               | SET_MAX  |           | Interface       | block
 * |               | SET_RES  |           |                 |
 *
 */
static bool set_feature_unit_control_error(struct usb_setup_data *req)
{
	int cs;
	int cn;

	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != (FEATURE_UNIT_ID << 8))
		return true;
	cs = req->wValue >> 8;
	cn = req->wValue & 0xff;
	if (cs == USB_AUDIO_MUTE_CONTROL) {
		if (req->bRequest != USB_AUDIO_SET_CUR || cn ||
		    req->wLength != 1)
			return true;
	} else if (cs == USB_AUDIO_VOLUME_CONTROL) {
		if (req->bRequest != USB_AUDIO_SET_CUR ||
		    (cn != 1 && cn != 2) ||
		    req->wLength != 2)
			return true;
	} else {
		return true;
	}
	return false;
}

static bool set_feature_unit_control_data_error(struct usb_setup_data *req,
						char *buf)
{
	int cs;
	short *p;

	cs = (req->wValue >> 8);
	if (cs == USB_AUDIO_VOLUME_CONTROL) {
		p = (short *)buf;
		if (*p > MAX_VOLUME || *p < MIN_VOLUME ||
		    *p & 255)	/* *p % RES_VOLUME  */
			return true;
	}
	return false;
}

static int request_set_feature_unit_control(struct usb_setup_data *req,
					    char *buf)
{
	int cs;
	int cn;
	short *p;
	char tmp[2];
	
	cs = (req->wValue >> 8);
	cn = (req->wValue & 0xff);
	if (cs == USB_AUDIO_MUTE_CONTROL) {
		if (*buf) {
			mute = true;
			ds1881_write(mute_data, 2);
		} else {
			mute = false;
			tmp[0] = (-volume[1] >> 8) & 0x3f;
			tmp[1] = ((-volume[2] >> 8) & 0x3f) | 0x40;
			ds1881_write(tmp, 2);
		}
	} else {
		p = (short *)buf;
		volume[cn] = *p;
		if (!mute) {
			tmp[0] = (-*p >> 8) & 0x3f;
			if (cn == 2)
				tmp[0] |= 0x40;
			ds1881_write(tmp, 1);
		}
	}
	return 0;
}

/*
 * Get Feature Unit Control Request
 *
 * | bmRequestType | bRequest | wValue    | wIndex          | wLength
 * | 10100001B     | GET_CUR  | CS and CN | Feature Unit ID | Length of
 * |               | GET_MIN  |           | and             | parameter
 * |               | GET_MAX  |           | Interface       | block
 * |               | GET_RES  |           |                 |
 *
 */
static bool get_feature_unit_control_error(struct usb_setup_data *req)
{
	int cs;
	int cn;

	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != (FEATURE_UNIT_ID << 8))
		return true;
	cs = req->wValue >> 8;
	cn = req->wValue & 0xff;
	if (cs == USB_AUDIO_MUTE_CONTROL) {
		if (req->bRequest != USB_AUDIO_GET_CUR || cn)
			return true;
	} else if (cs == USB_AUDIO_VOLUME_CONTROL) {
		if (req->bRequest != USB_AUDIO_GET_CUR &&
		    req->bRequest != USB_AUDIO_GET_MIN &&
		    req->bRequest != USB_AUDIO_GET_MAX &&
		    req->bRequest != USB_AUDIO_GET_RES)
			return true;
		if (cn != 1 && cn != 2)
			return true;
	} else {
		return true;
	}
	return false;
}

static int request_get_feature_unit_control(struct usb_setup_data *req,
					    char *buf, char **data)
{
	int cs;
	int cn;
	short *p;
	int len;

	cs = (req->wValue >> 8);
	cn = (req->wValue & 0xff);
	if (cs == USB_AUDIO_MUTE_CONTROL) {
		*buf = mute;
		*data = buf;
		len = 1;
	} else {
		p = (short *)buf;
		switch (req->bRequest) {
		case USB_AUDIO_GET_CUR:
			*p = volume[cn];
			break;
		case USB_AUDIO_GET_MIN:
			*p = MIN_VOLUME;
			break;
		case USB_AUDIO_GET_MAX:
			*p = MAX_VOLUME;
			break;
		case USB_AUDIO_GET_RES:
		default:
			*p = RES_VOLUME;
			break;
		}
		*data = buf;
		len = 2;
	}
	return len;
}

bool class_request_error(struct usb_setup_data *req)
{
	if (req->bmRequestType & USB_DIR_IN)
		return get_feature_unit_control_error(req);
	else
		return set_feature_unit_control_error(req);
}

bool class_request_data_error(struct usb_setup_data *req, char *buf)
{
	return set_feature_unit_control_data_error(req, buf);
}

int class_request(struct usb_setup_data *req, char *buf, char **data)
{
	if (req->bmRequestType & USB_DIR_IN)
		return request_get_feature_unit_control(req, buf, data);
	else
		return request_set_feature_unit_control(req, buf);
}

void class_reset(void)
{
	volume[1] = DEFAULT_VOLUME;
	volume[2] = DEFAULT_VOLUME;
	mute = true;
	ds1881_write(mute_data, 2);
}
