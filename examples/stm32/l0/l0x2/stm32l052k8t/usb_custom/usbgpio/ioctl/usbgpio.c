/*
 * usbgpio
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/usb/ch9.h>
#include <linux/usbdevice_fs.h>
#include <unistd.h>

#define GPIO_DEV "/dev/ugpio"
#define GET_GPIOA 0
#define SET_GPIOA 1

int main(int argc, char *argv[])
{
	unsigned short data;
	int fd;
	struct usbdevfs_ctrltransfer udc;

	if (argc > 1) {
		if (sscanf(argv[1], "%hx", &data) != 1) {
			fprintf(stderr, "Invalid argument: %s\n", argv[1]);
			return 1;
		}
	}

	if ((fd = open(GPIO_DEV, O_RDWR, 0)) == -1) {
		fprintf(stderr, "%s: ", GPIO_DEV);
		perror("open() failed");
		return 1;
	}

	if (argc < 2) {
		udc.bRequestType = (USB_DIR_IN | USB_TYPE_VENDOR |
				    USB_RECIP_DEVICE);
		udc.bRequest = GET_GPIOA;
		udc.wValue = 0;
		udc.wIndex = 0;
		udc.wLength = sizeof(data);
		udc.timeout = 1000;
		udc.data = &data;
		if (ioctl(fd, USBDEVFS_CONTROL, &udc) == -1) {
			perror("ioctl() (USBDEVFS_CONTROL) failed");
			close(fd);
			return 1;
		}
		printf("Port: 0x%04x\n", data);
	} else {
		udc.bRequestType = (USB_DIR_OUT | USB_TYPE_VENDOR |
				    USB_RECIP_DEVICE);
		udc.bRequest = SET_GPIOA;
		udc.wValue = data;
		udc.wIndex = 0;
		udc.wLength = 0;
		udc.timeout = 1000;
		udc.data = NULL;
		if (ioctl(fd, USBDEVFS_CONTROL, &udc) == -1) {
			perror("ioctl() (USBDEVFS_CONTROL) failed");
			close(fd);
			return 1;
		}
		printf("Port <- 0x%04x\n", data);
	}

	if (close(fd) == -1) {
		perror("close() failed");
		return 1;
	}
	return 0;
}
