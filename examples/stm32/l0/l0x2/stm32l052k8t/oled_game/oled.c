/*
 * oled.c - Solomon Systech SSD1306 OLED Controller
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of oled_game.
 *
 * oled_game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * oled_game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with oled_game.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <gpio.h>
#include <spi.h>

#include "oled_game.h"

/*
 * SparkFun Micro OLED Breakout (LCD-13003)
 *
 * CS# - PA6, D/C# - PA2, RES# - PA3
 */
/* SSD1306 command */
#define OLED_SET_CONTRAST		0x81
#define OLED_RESUME_DISPLAY		0xa4
#define OLED_ENTIRE_DISPLAY_ON		0xa5
#define OLED_SET_NORMAL_DISPLAY		0xa6
#define OLED_SET_INVERSE_DISPLAY	0xa7
#define OLED_SET_DISPLAY_OFF		0xae
#define OLED_SET_DISPLAY_ON		0xaf

#define OLED_SCROLL_H_RIGHT		0x26
#define OLED_SCROLL_H_LEFT		0x27
#define OLED_SCROLL_V_H_RIGHT		0x29
#define OLED_SCROLL_V_H_LEFT		0x2a
#define OLED_DEACTIVATE_SCROLL		0x2e
#define OLED_ACTIVATE_SCROLL		0x2f
#define OLED_SET_V_SCROLL_AREA		0xa3

#define OLED_SET_LOWER_COLUMN_START	0x00
#define OLED_SET_HIGHER_COLUMN_START	0x10
#define OLED_SET_MEMORY_ADDRESSING_MODE	0x20
#define OLED_SET_COLUMN_ADDRESS		0x21
#define OLED_SET_PAGE_ADDRESS		0x22
#define OLED_SET_PAGE_START		0xb0

#define OLED_SET_DISPLAY_START_LINE	0x40
#define OLED_SET_SEGMENT_REMAP_0	0xa0
#define OLED_SET_SEGMENT_REMAP_127	0xa1
#define OLED_SET_MUX_RATIO		0xa8
#define OLED_SET_COM_SCAN_NORMAL	0xc0
#define OLED_SET_COM_SCAN_REMAPPED	0xc8
#define OLED_SET_DISPLAY_OFFSET		0xd3
#define OLED_SET_COM_PINS		0xda

#define OLED_SET_DCLK_FOSC		0xd5
#define OLED_SET_PRECHARGE_PERIOD	0xd9
#define OLED_SET_VCOMH_DESELECT_LEVEL	0xdb
#define OLED_NOP			0xe3

#define OLED_CHARGE_PUMP		0x8d

#define OLED_TRESET			3 /* Reset low pulse width: 3 us */

#define FONT_WIDTH			6 /* 6x8 */

/*
 * Frame buffer
 *   x=0 .. 63
 * D0         page0 y=0
 * :
 * D7
 *            :
 *            page5
 *                  y=47
 */
static char fbuf[MAXY / 8][MAXX];

static void oled_send_command(int command)
{
	gpio_clear(GPIO_PA(6, 2));
	spi_send_blocking(SPI1, command);
	spi_recv_blocking(SPI1);
	gpio_set(GPIO_PA6);
}

static void oled_send_data(int data)
{
	gpio_clear(GPIO_PA6);
	gpio_set(GPIO_PA2);
	spi_send_blocking(SPI1, data);
	spi_recv_blocking(SPI1);
	gpio_set(GPIO_PA6);
}

static void oled_set_address(int page, int column)
{
	oled_send_command(OLED_SET_PAGE_START | (page & 7));
	oled_send_command(OLED_SET_HIGHER_COLUMN_START |
			  (((column >> 4) + 2) & 0xf));
	oled_send_command(OLED_SET_LOWER_COLUMN_START | (column & 0xf));
}

void oled_init(void)
{
	int y;
	int x;
	int d;
	
	delay_us(OLED_TRESET);

	gpio_set(GPIO_PA3);
	oled_send_command(OLED_SET_MUX_RATIO);
	oled_send_command(0x2f);

	oled_send_command(OLED_CHARGE_PUMP);
	oled_send_command(0x14);

	oled_send_command(OLED_SET_PRECHARGE_PERIOD);
	oled_send_command(0xf1);

	oled_send_command(OLED_SET_VCOMH_DESELECT_LEVEL);
	oled_send_command(0x40);

	oled_send_command(OLED_SET_DISPLAY_ON);

	for (y = 0; y < MAXY / 8; y++) {
		for (x = 0; x < MAXX; x++) {
			if (x == 0)
				d = 0xff;
			else if (x == MAXX - 1)
				d = 0;
			else if (y == 0)
				d = 1;
			else if (y == MAXY / 8 - 1)
				d = 0x80;
			else
				d = 0;
			fbuf[y][x] = d;
		}
	}
	
	for (y = 0; y < MAXY / 8; y++) {
		oled_set_address(y, 0);
		for (x = 0; x < MAXX; x++)
			oled_send_data(fbuf[y][x]);
	}
}

void oled_point(int x, int y, int color)
{
	if (color)
		fbuf[y / 8][x] |= 1 << (y & 7);
	else
		fbuf[y / 8][x] &= ~(1 << (y & 7));

	oled_set_address(y / 8, x);
	oled_send_data(fbuf[y / 8][x]);
}

void oled_line_y(int x, int y, int length, int color)
{
	int n;
	int i;
	
	while (length) {
		n = (length < 8 - (y & 7)) ? length : (8 - (y & 7));
		for (i = 0; i < n; i++) {
			if (color)
				fbuf[y / 8][x] |= 1 << ((y & 7) + i);
			else
				fbuf[y / 8][x] &= ~(1 << ((y & 7) + i));
		}

		oled_set_address(y / 8, x);
		oled_send_data(fbuf[y / 8][x]);

		y += n;
		length -= n;
	}
}

void oled_string(int page, int column, char *str)
{
	int i;
	extern const char font[];
	
	oled_set_address(page, column);
	while (*str) {
		for (i = 0; i < FONT_WIDTH; i++)
			oled_send_data(font[(*str - ' ') * FONT_WIDTH + i]);
		str++;
	}
}
