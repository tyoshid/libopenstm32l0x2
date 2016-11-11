/*
 * usbgpio
 *
 * Copyright 2013 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#define VID 0x2975
#define PID 0x0001
#define GET_GPIOA 0
#define SET_GPIOA 1

int main(int argc, char *argv[])
{
	unsigned short data;
	int r;
	libusb_context *ctx = NULL;
	int i;
	libusb_device_handle *dev_handle;

	if (argc > 1) {
		if (sscanf(argv[1], "%hx", &data) != 1) {
			fprintf(stderr, "Invalid argument: %s\n", argv[1]);
			return 1;
		}
	}

	r = libusb_init(&ctx);
	if (r < 0) {
		fprintf(stderr, "libusb_init() failed: %s\n",
			libusb_error_name(r));
		return 1;
	}
	//libusb_set_debug(ctx, 3);

	for (i = 0; i < 3; i++) {
		dev_handle = libusb_open_device_with_vid_pid(ctx, VID, PID);
		if (dev_handle)
			break;
		usleep(100000);
	}
	if (!dev_handle) {
		fprintf(stderr, "libusb_open_device_with_vid_pid() failed.\n");
		libusb_exit(ctx);
		return 1;
	}

	if (argc < 2) {
		r = libusb_control_transfer(dev_handle,
					    LIBUSB_REQUEST_TYPE_VENDOR |
					    LIBUSB_RECIPIENT_DEVICE |
					    LIBUSB_ENDPOINT_IN, GET_GPIOA, 0,
					    0, (unsigned char *)&data,
					    sizeof(data), 1000);
		if (r < 0) {
			fprintf(stderr, "libusb_control_transfer() failed: "
				"%s\n", libusb_error_name(r));
			libusb_close(dev_handle);
			libusb_exit(ctx);
			return 1;
		}
		printf("Port: 0x%04x\n", data);
	} else {
		r = libusb_control_transfer(dev_handle,
					    LIBUSB_REQUEST_TYPE_VENDOR |
					    LIBUSB_RECIPIENT_DEVICE |
					    LIBUSB_ENDPOINT_OUT, SET_GPIOA,
					    data, 0, NULL, 0, 1000);
		if (r < 0) {
			fprintf(stderr, "libusb_control_transfer() failed: "
				"%s\n", libusb_error_name(r));
			libusb_close(dev_handle);
			libusb_exit(ctx);
			return 1;
		}
		printf("Port <- 0x%04x\n", data);
	}

	libusb_close(dev_handle);
	libusb_exit(ctx);
	return 0;
}
