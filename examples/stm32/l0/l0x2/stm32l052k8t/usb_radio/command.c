/*
 * command.c
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>

#include <nvic.h>
#include <vector.h>

#include "usb_radio.h"

#define MAXTOKEN		10
#define BUFSIZE			64
#define DELIMITER		" /,;:"
#define PROMPT			"usb_radio> "

static char buf[BUFSIZE];
static int bufindex;

static void tune(int n, char **p)
{
	double d;
	int f;
	char *endp;
	
	if (n != 1) {
		puts("Invalid argument: <64.00-108.00>");
		return;
	}
	d = strtod(*p, &endp);
	if (*endp != '\0' || d < 64. || d > 108.) {
		printf("Invalid value: %s (64.00-108.00)\n", *p);
		return;
	}
	f = d * 100.;
	si4737_tune(f);
}

static void chip(void)
{
	si4737_revision();
}

static void signal(void)
{
	si4737_rsq();
}

static void debug(void)
{
	extern volatile unsigned int send_packet;
	extern volatile unsigned int send_len;
	extern volatile unsigned int tim2_enable;
	extern volatile unsigned int overrun;
	extern volatile int i2c_status;
	extern volatile int spi_status;
	extern volatile int usbdevfs_status;
	extern volatile int si4737_status;
	extern volatile int apb2clk;
	extern volatile int hclk;
	unsigned int tmp[10];
	
	nvic_disable_irq(NVIC_USB);
	tmp[0] = send_packet;
	tmp[1] = send_len;
	tmp[2] = tim2_enable;
	tmp[3] = overrun;
	tmp[4] = i2c_status;
	tmp[5] = spi_status;
	tmp[6] = usbdevfs_status;
	tmp[7] = si4737_status;
	tmp[8] = apb2clk;
	tmp[9] = hclk;
	nvic_enable_irq(NVIC_USB);

	printf("send_packet: %u\n", tmp[0]);
	printf("send_len: %u\n", tmp[1]);
	printf("tim2_enable: %u\n", tmp[2]);
	printf("overrun: %u\n", tmp[3]);
	printf("i2c_status: 0x%08x\n", tmp[4]);
	printf("spi_status: 0x%04x\n", tmp[5]);
	printf("usbdevfs_status: 0x%04x\n", tmp[6]);
	printf("si4737_status: 0x%02x\n", tmp[7]);
	printf("apb2clk: %u\n", tmp[8]);
	printf("hclk: %u\n", tmp[9]);
}

static void help(void)
{
	puts("Usage:");
	puts("  <64.00-108.00>  Tune a frequency (64-108 MHz)");
	puts("  chip            Print the part number");
	puts("  signal          Print the Received Signal Quality");
	puts("  debug           Print statistics");
	puts("  help            Print this message");
}

void command_init(void)
{
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	printf(PROMPT);
	bufindex = 0;
}

static void command(void)
{
	int i;
	static char *p[MAXTOKEN];

	for (i = 0; i < MAXTOKEN; i++) {
		p[i] = strtok(i ? NULL : buf, DELIMITER);
		if (p[i] == NULL)
			break;
	}
	if (i) {
		if (isdigit(*p[0]))
			tune(i, p);
		else if (strncasecmp(p[0], "chip", strlen(p[0])) == 0)
			chip();
		else if (strncasecmp(p[0], "signal", strlen(p[0])) == 0)
			signal();
		else if (strncasecmp(p[0], "debug", strlen(p[0])) == 0)
			debug();
		else if (strncasecmp(p[0], "help", strlen(p[0])) == 0 ||
			 strcmp(p[0], "?") == 0)
			help();
		else
			puts("unknown command");
	}
}

void command_line(void)
{
	int c;

	c = getchar();
	if (isprint(c)) {
		putchar(c);
		if (bufindex < BUFSIZE - 1)
			buf[bufindex++] = c;
	} else {
		switch (c) {
		case '\b':
			if (bufindex > 0) {
				printf("\b \b");
				--bufindex;
			}
			break;
		case '\n':
			putchar('\n');
			buf[bufindex] = '\0';
			command();
			printf(PROMPT);
			bufindex = 0;
			break;
		default:
			break;
		}
	}
}
