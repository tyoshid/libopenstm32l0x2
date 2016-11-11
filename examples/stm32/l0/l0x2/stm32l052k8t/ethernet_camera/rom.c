/*
 * rom.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of ethernet_camera.
 *
 * ethernet_camera is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ethernet_camera is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ethernet_camera.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <i2c.h>

/*
 * Atmel AT24MAC402-ST
 * I2C-Compatible(2-wire) 2-Kbit(256 x 8) Serial EEPROM
 * with a Factory-Programmed EUI-48 Address
 * Plus an Embedded Unique 128-bit Serial Number
 */
#define ROM_SLA_EEPROM		0xa0
#define ROM_SLA_EUI		0xb0
#define ROM_EUI48_ADDR		0x9a

int rom_read_eui48(char *buf)
{
	int r;
	int i;
	
	i2c_start(I2C1, ROM_SLA_EUI, 1);
	i2c_send_blocking(I2C1, ROM_EUI48_ADDR);
	while (!i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					 I2C_NACKF | I2C_TC))
			;
	r = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR | I2C_NACKF);
	if (r)
		return r;

	i2c_start(I2C1, I2C_AUTOEND | I2C_READ | ROM_SLA_EUI, 6);
	for (i = 0; i < 6; i++) {
		*(buf + i) = i2c_recv_blocking(I2C1);
		r = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					     I2C_NACKF);
		if (r)
			return r;
	}

	while (!i2c_get_interrupt_status(I2C1, I2C_BERR | I2C_STOPF))
		;
	r = i2c_get_interrupt_status(I2C1, I2C_BERR);
	if (r)
		return r;
	i2c_clear_interrupt(I2C1, I2C_STOPF);
	return 0;
}
