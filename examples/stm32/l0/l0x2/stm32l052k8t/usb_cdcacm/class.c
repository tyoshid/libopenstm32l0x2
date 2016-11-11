/*
 * class.c
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

#include <gpio.h>
#include <tim.h>
#include <usart.h>

#include "usb_cdcacm.h"

void notify_response_available(void);
void notify_reset(void);
void bulk_reset(void);
enum v25ter_result v25ter_input_encap(int c, char **info);
void v25ter_reset(void);

#define MAXRESPONSESIZE 32

alignas(2) static char response[MAXRESPONSESIZE];
static int response_len;
alignas(2) struct usb_cdc_line_coding usart_param;
int databits = 8;
bool parity_mark;
static short control_line_state;

/*
 * Send Encapsulated Command
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength   | Data
 * | 00100001B     | 00h      | Zero   | Interface | Amount of | Control
 * |               |          |        |           | Data      | protocol-based
 * |               |          |        |           |           | command
 *
 */
static bool send_encapsulated_command_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wValue || !req->wLength)
		return true;
	return false;
}

static int request_send_encapsulated_command(struct usb_setup_data *req,
					     char *buf)
{
	int i;
	int r;
	char *info;

	for (i = 0; i < req->wLength; i++) {
		r = v25ter_input_encap(*buf++, &info);
		if (r != RESULT_NONE)
			break;
	}
	if (r != RESULT_NONE) {
		for (i = 0; i < MAXRESPONSESIZE; i++) {
			if (!*info)
				break;
			response[i] = *info++;
		}
		response_len = i;
		notify_response_available();
	}
	return 0;
}

/*
 * Get Encapsulated Response
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength   | Data
 * | 10100001B     | 01h      | Zero   | Interface | Amount of | Protocol-
 * |               |          |        |           | Data      | dependent
 * |               |          |        |           |           | data
 *
 */
static bool get_encapsulated_response_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
			return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wValue || !req->wLength)
		return true;
	return false;
}

static int request_get_encapsulated_response(struct usb_setup_data *req,
					     char **data)
{
	*data = response;
	if (response_len > req->wLength)
		return req->wLength;
	return response_len;
}

/*
 * Set Line Coding
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength   | Data
 * | 00100001B     | 20h      | Zero   | Interface | Size of   | Line Coding
 * |               |          |        |           | Structure | Structure
 *
 */
static bool set_line_coding_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wValue || req->wLength != sizeof(struct usb_cdc_line_coding))
		return true;
	return false;
}

static bool set_line_coding_data_error(char *buf)
{
	struct usb_cdc_line_coding *p;

	p = (struct usb_cdc_line_coding *)buf;
	if (p->bCharFormat > 2)
		return true;
	if (p->bParityType > 4)
		return true;
	if (p->bDataBits < 6 || p->bDataBits > 8)
		return true;
	if (p->bDataBits == 6 &&
	    p->bParityType == USB_CDC_LINE_CODING_PARITYTYPE_NONE)
		return true;
	return false;
}

static int request_set_line_coding(char *buf)
{
	struct usb_cdc_line_coding *p;
	static const enum usart_stop stop[] = {
		USART_STOP_1,
		USART_STOP_1_5,
		USART_STOP_2
	};
	static const enum usart_parity parity[] = {
		USART_PARITY_NONE,
		USART_ODD,
		USART_EVEN,
		USART_PARITY_NONE,	/* Mark */
		USART_PARITY_NONE	/* Space */
	};

	p = (struct usb_cdc_line_coding *)buf;
	usart_param = *p;
	databits = p->bDataBits;
	parity_mark = (p->bParityType == USB_CDC_LINE_CODING_PARITYTYPE_MARK);
	usart_set_txrx(USART2, USART_TXRX_DISABLE, 0);
	while (usart_get_interrupt_status(USART2, USART_TEACK | USART_REACK))
		;
	usart_disable(USART2);
	if (p->dwDTERate) {
		if (p->bDataBits == 6 ||
		    (p->bDataBits == 7 &&
		     p->bParityType == USB_CDC_LINE_CODING_PARITYTYPE_NONE))
			usart_set_databits(USART2, 7);
		else if (p->bDataBits == 7 ||
			 (p->bDataBits == 8 &&
			  p->bParityType ==
			  USB_CDC_LINE_CODING_PARITYTYPE_NONE))
			usart_set_databits(USART2, 8);
		else
			usart_set_databits(USART2, 9);
		usart_set_baudrate(USART2, FCK, p->dwDTERate);
		usart_set_stopbits(USART2, stop[p->bCharFormat]);
		usart_set_parity(USART2, parity[p->bParityType]);
		usart_enable(USART2);
		usart_set_txrx(USART2, USART_TX_RX, 0);
	}
	return 0;
}

/*
 * Get Line Coding
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength   | Data
 * | 10100001B     | 21h      | Zero   | Interface | Size of   | Line Coding
 * |               |          |        |           | Structure | Structure
 *
 */
static bool get_line_coding_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_IN | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wValue || req->wLength != sizeof(struct usb_cdc_line_coding))
		return true;
	return false;
}

static int request_get_line_coding(char **data)
{
	*data = (char *)&usart_param;
	return sizeof(struct usb_cdc_line_coding);
}

/*
 * Set Control Line State
 *
 * | bmRequestType | bRequest | wValue | wIndex    | wLength
 * | 00100001B     | 22h      | Bitmap | Interface | Zero
 *
 */
static bool set_control_line_state_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wLength)
		return true;
	return false;
}

static int request_set_control_line_state(struct usb_setup_data *req)
{
	control_line_state = req->wValue;
	return 0;
}

/*
 * Send Break
 *
 * | bmRequestType | bRequest | wValue   | wIndex    | wLength
 * | 00100001B     | 23h      | Duration | Interface | Zero
 *
 */
static bool send_break_error(struct usb_setup_data *req)
{
	if (req->bmRequestType != (USB_DIR_OUT | USB_TYPE_CLASS |
				   USB_RECIP_INTERFACE))
		return true;
	if (req->wIndex != INTERFACE_COMM)
		return true;
	if (req->wLength)
		return true;
	return false;
}

static int request_send_break(struct usb_setup_data *req)
{
	if (req->wValue) {
		gpio_set_mode(GPIO_OUTPUT, GPIO_PA2);
		if (req->wValue < 32768) {
			tim_setup_counter(TIM22, TIMX_CLK_APB2 / 2000,
					   req->wValue << 1);
			tim_enable_counter(TIM22);
		} else if (req->wValue != 0xffff) {
			tim_setup_counter(TIM22, TIMX_CLK_APB2 / 1000,
					   req->wValue);
			tim_enable_counter(TIM22);
		}
	} else {
		tim_disable_counter(TIM22);
		gpio_set_mode(GPIO_ALTFUNC, GPIO_PA2);
	}
	return 0;
}

bool class_request_error(struct usb_setup_data *req)
{
	switch (req->bRequest) {
	case USB_CDC_REQ_SEND_ENCAPSULATED_COMMAND:
		return send_encapsulated_command_error(req);
	case USB_CDC_REQ_GET_ENCAPSULATED_RESPONSE:
		return get_encapsulated_response_error(req);
	case USB_CDC_REQ_SET_LINE_CODING:
		return set_line_coding_error(req);
	case USB_CDC_REQ_GET_LINE_CODING:
		return get_line_coding_error(req);
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		return set_control_line_state_error(req);
	case USB_CDC_REQ_SEND_BREAK:
		return send_break_error(req);
	default:
		break;
	}
	return true;
}

bool class_request_data_error(struct usb_setup_data *req, char *buf)
{
	if (req->bRequest == USB_CDC_REQ_SET_LINE_CODING)
		return set_line_coding_data_error(buf);
	else if (req->bRequest == USB_CDC_REQ_SEND_ENCAPSULATED_COMMAND)
		return false;
	return true;
}

int class_request(struct usb_setup_data *req, char *buf, char **data)
{
	switch (req->bRequest) {
	case USB_CDC_REQ_SEND_ENCAPSULATED_COMMAND:
		return request_send_encapsulated_command(req, buf);
	case USB_CDC_REQ_GET_ENCAPSULATED_RESPONSE:
		return request_get_encapsulated_response(req, data);
	case USB_CDC_REQ_SET_LINE_CODING:
		return request_set_line_coding(buf);
	case USB_CDC_REQ_GET_LINE_CODING:
		return request_get_line_coding(data);
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		return request_set_control_line_state(req);
	case USB_CDC_REQ_SEND_BREAK:
		return request_send_break(req);
	default:
		break;
	}
	return -1;
}

void class_reset(void)
{
	notify_reset();
	bulk_reset();
	v25ter_reset();
}
