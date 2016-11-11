/*
 * cdcmng
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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

#define MAXPATH			9
#define BUFSIZE			64
#define INTERFACE_COMM		0
#define NOTIFICATION_ENUM	0x81

bool init;
libusb_device **list;
libusb_device_handle *dev_handle;
bool detach;
bool claim;
char buf[BUFSIZE + 1];

static int cdcmng_open(int *path, int pathlen)
{
	int r;
	int cnt;
	int i;
	uint8_t port[MAXPATH - 1];
	int j;

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(stderr, "libusb_init() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	init = true;
	cnt = libusb_get_device_list(NULL, &list);
	if (cnt < 0) {
		fprintf(stderr, "libusb_get_device_list() failed: %s\n",
			libusb_error_name(cnt));
		return -1;
	}
	for (i = 0; i < cnt; i++) {
		r = libusb_get_bus_number(list[i]);
		if (r != *path)
			continue;
		r = libusb_get_port_numbers(list[i], port, sizeof(port));
		if (r < 0) {
			fprintf(stderr, "libusb_get_port_numbers() failed: "
				"%s\n", libusb_error_name(r));
			return -1;
		}
		if (r != pathlen - 1)
			continue;
		for (j = 0; j < r; j++) {
			if (port[j] != *(path + j + 1))
				break;
		}
		if (j >= r)
			break;
	}
	if (i >= cnt) {
		fprintf(stderr, "device not found\n");
		return -1;
	}
	r = libusb_open(list[i], &dev_handle);
	if (r < 0) {
		fprintf(stderr, "libusb_open() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	r = libusb_detach_kernel_driver(dev_handle, INTERFACE_COMM);
	if (r < 0) {
		fprintf(stderr, "libusb_detach_kernel_driver() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	detach = true;
	r = libusb_claim_interface(dev_handle, INTERFACE_COMM);
	if (r < 0) {
		fprintf(stderr, "libusb_claim_interface() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	claim = true;
	return 0;
}

int cdcmng_command(char *command)
{
	int r;
	int len;
	
	sprintf(buf, "%s\r\n", command);
	/* SEND_ENCAPSULATED_COMMAND */
	r = libusb_control_transfer(dev_handle,
				    LIBUSB_REQUEST_TYPE_CLASS |
				    LIBUSB_RECIPIENT_INTERFACE |
				    LIBUSB_ENDPOINT_OUT, 0x00, 0,
				    INTERFACE_COMM, (unsigned char *)buf,
				    strlen(buf), 1000);
	if (r < 0) {
		fprintf(stderr, "libusb_control_transfer() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	printf("%s", command);
	/* RESPONSE_AVAILABLE */
	do {
		r = libusb_interrupt_transfer(dev_handle, NOTIFICATION_ENUM,
					      (unsigned char *)buf, BUFSIZE,
					      &len, 1000);
		if (r < 0) {
			fprintf(stderr, "libusb_interrupt_transfer() failed: "
				"%s\n", libusb_error_name(r));
			return -1;
		}
	} while (len < 2 || buf[1] != 0x01);
	/* GET_ENCAPSULATED_RESPONSE */
	r = libusb_control_transfer(dev_handle,
				    LIBUSB_REQUEST_TYPE_CLASS |
				    LIBUSB_RECIPIENT_INTERFACE |
				    LIBUSB_ENDPOINT_IN, 0x01, 0,
				    INTERFACE_COMM, (unsigned char *)buf,
				    BUFSIZE, 1000);
	if (r < 0) {
		fprintf(stderr, "libusb_control_transfer() failed: %s\n",
			libusb_error_name(r));
		return -1;
	}
	buf[r] = '\0';
	printf("%s", buf);
	if (strstr(buf, "\r\nERROR\r\n")) {
		fprintf(stderr, "command failed\n");
		return -1;
	}
	return 0;
}

void cdcmng_close(void)
{
	if (claim)
		libusb_release_interface(dev_handle, INTERFACE_COMM);
	if (detach)
		libusb_attach_kernel_driver(dev_handle, INTERFACE_COMM);
	if (dev_handle)
		libusb_close(dev_handle);
	if (list)
		libusb_free_device_list(list, 1);
	if (init)
		libusb_exit(NULL);
}

int main(int argc, char *argv[])
{
	int i;
	char *p;
	int path[MAXPATH];
	char *endp;
	int pathlen;
	
	if (argc < 3) {
		printf("%s <device-path> <command>\n", argv[0]);
		return 1;
	}
	for (i = 0; i < MAXPATH; i++) {
		p = strtok(i == 0 ? argv[1] : NULL, "-.");
		if (!p)
			break;
		path[i] = strtol(p, &endp, 10);
		if (*endp != '\0') {
			fprintf(stderr, "invalid device-path\n");
			return 1;
		}
	}
	if (i >= MAXPATH) {
		fprintf(stderr, "device-path too long\n");
		return 1;
	}
	pathlen = i;
	if (strlen(argv[2]) > BUFSIZE - 3) {
		fprintf(stderr, "command too long\n");
		return 1;
	}
	if (cdcmng_open(path, pathlen)) {
		cdcmng_close();
		return 1;
	}
	if (cdcmng_command(argv[2])) {
		cdcmng_close();
		return 1;
	}
	cdcmng_close();
	return 0;
}
