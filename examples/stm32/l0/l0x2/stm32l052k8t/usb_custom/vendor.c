/*
 * vendor.c
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

#include <gpio.h>
#include <usbdevfs.h>

/*
 * This USB device has GPIOA read/write vendor requests.
 * GPIO0 is connected to LED1, GPIO1 is connected to LED2.
 * If you set bit 1 (0x0002), LED2 is turned off.
 *
 * | bmRequestType | bRequest | wValue      | wIndex | wLength | Data
 * | 11000000B     | 0(read)  | Zero        | Zero   | Two     | GPIOA Value
 * | 01000000B     | 1(write) | GPIOA Value | Zero   | Zero    | None
 *
 */

/* Vendor-specific Device Requests */
int vendor_request(struct usb_setup_data *req)
{
	short data;

	switch (req->bRequest) {
	case 0:			/* GET_GPIOA */
		data = gpio_read_port(GPIOA);
		usbdevfs_write(0, &data, sizeof(data));
		break;
	case 1:			/* SET_GPIOA */
		gpio_write_port(GPIOA, req->wValue);
		break;
	default:
		return -1;
	}

	return 0;
}
