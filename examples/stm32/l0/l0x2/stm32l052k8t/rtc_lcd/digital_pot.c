/*
 * digital_pot.c - SPI Digital Potentiometer driver
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of rtc_lcd.
 *
 * rtc_lcd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtc_lcd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rtc_lcd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>

#include <gpio.h>
#include <spi.h>

/*
 * Microchip MCP4131
 * 7-Bit Single SPI Digital POT with Volatile Memory
 * (SparkFun COM-10613)
 */
#define MCP_WR_VOLATILE_WIPER0	0x02
#define MCP_RD_VOLATILE_WIPER0	0x0f
#define MCP_INC_VOLATILE_WIPER0	0x04
#define MCP_DEC_VOLATILE_WIPER0	0x08
#define MCP_WR_VOLATILE_TCON	0x42
#define MCP_RD_VOLATILE_TCON	0x4f
#define MCP_RD_STATUS		0x5f

void mcp_write_wiper(int w)
{
	int r[2];

	gpio_clear(GPIO_PB6);
	spi_send_blocking(SPI1, MCP_WR_VOLATILE_WIPER0);
	r[0] = spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, w);
	r[1] = spi_recv_blocking(SPI1);
	gpio_set(GPIO_PB6);
	if (r[0] != MCP_WR_VOLATILE_WIPER0 || r[1] != w)
		fputs("MCP4131 command error!", stderr);
}
