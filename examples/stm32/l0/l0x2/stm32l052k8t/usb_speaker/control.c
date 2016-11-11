/*
 * control.c
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
#include <stdalign.h>

#include <usb/standard.h>

#include <usbdevfs.h>

#include "usb_speaker.h"

void standard_reset(void);
int usb_device_request(struct usb_setup_data *req, enum control_stage stage,
		       char *buf, char **data);

static enum usbdevfs_control_state state;
static struct usb_setup_data curreq; /* Current device request */
alignas(2) static char inbuf[MAX_DATA_IN];
static char *inp;		/* IN data pointer */
static int inlen;		/* IN data length */
alignas(2) static char outbuf[MAX_DATA_OUT];
static char *outp;		/* OUT data pointer */
static int outlen;		/* OUT data length */
static bool in_zero_length_packet;

/* SETUP transaction */
void control_setup(void)
{
	int n;

	if (usbdevfs_get_rx_count(0) != sizeof(struct usb_setup_data)) {
		state = USBDEVFS_STALL;
		usbdevfs_set_control_state(0, state);
		return;
	}
	usbdevfs_read(0, (char *)&curreq, sizeof(curreq));
	inlen = usb_device_request(&curreq, CONTROL_STAGE_SETUP, inbuf, &inp);
	if (inlen < 0) {
		state = USBDEVFS_STALL;
		usbdevfs_set_control_state(0, state);
		return;
	}
	if (curreq.bmRequestType & USB_DIR_IN) {
		in_zero_length_packet = (inlen < curreq.wLength &&
					 (inlen & (MAXPACKETSIZE0 - 1)) == 0);
		if (inlen < MAXPACKETSIZE0)
			n = inlen;
		else
			n = MAXPACKETSIZE0;
		usbdevfs_write(0, inp, n);
		inp += n;
		inlen -= n;
		if (n < MAXPACKETSIZE0 ||
		    (inlen == 0 && !in_zero_length_packet))
			state = USBDEVFS_LAST_DATA_IN;
		else
			state = USBDEVFS_DATA_IN;
		usbdevfs_set_control_state(0, state);
	} else {
		if (curreq.wLength) {
			outp = outbuf;
			outlen = curreq.wLength;
			if (curreq.wLength < MAXPACKETSIZE0)
				state = USBDEVFS_LAST_DATA_OUT;
			else
				state = USBDEVFS_DATA_OUT;
			usbdevfs_set_control_state(0, state);
		} else {
			usbdevfs_write(0, 0, 0);
			state = USBDEVFS_STATUS_IN;
			usbdevfs_set_control_state(0, state);
		}
	}
}

/* OUT transaction */
void control_rx(void)
{
	int len;

	len = usbdevfs_get_rx_count(0);
	if ((state == USBDEVFS_DATA_OUT && len != MAXPACKETSIZE0) ||
	    (state == USBDEVFS_LAST_DATA_OUT && len != outlen)) {
		state = USBDEVFS_STALL;
		usbdevfs_set_control_state(0, state);
		return;
	}
	usbdevfs_read(0, outp, outlen);
	switch (state) {
	case USBDEVFS_DATA_OUT:
		outp += len;
		outlen -= len;
		if (outlen <= MAXPACKETSIZE0)
			state = USBDEVFS_LAST_DATA_OUT;
		else
			state = USBDEVFS_DATA_OUT;
		break;
	case USBDEVFS_LAST_DATA_OUT:
		if (usb_device_request(&curreq, CONTROL_STAGE_DATA, outbuf,
				       0)) {
			state = USBDEVFS_STALL;
			break;
		}
		usbdevfs_write(0, 0, 0);
		state = USBDEVFS_STATUS_IN;
		break;
	case USBDEVFS_STATUS_OUT:
	default:
		state = USBDEVFS_STALL;
		break;
	}
	usbdevfs_set_control_state(0, state);
}

/* IN transaction */
void control_tx(void)
{
	int n;

	switch (state) {
	case USBDEVFS_DATA_IN:
		if (inlen < MAXPACKETSIZE0)
			n = inlen;
		else
			n = MAXPACKETSIZE0;
		usbdevfs_write(0, inp, n);
		inp += n;
		inlen -= n;
		if (n < MAXPACKETSIZE0 ||
		    (inlen == 0 && !in_zero_length_packet))
			state = USBDEVFS_LAST_DATA_IN;
		else
			state = USBDEVFS_DATA_IN;
		break;
	case USBDEVFS_LAST_DATA_IN:
		state = USBDEVFS_STATUS_OUT;
		break;
	case USBDEVFS_STATUS_IN:
		usb_device_request(&curreq, CONTROL_STAGE_STATUS, outbuf, 0);
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
