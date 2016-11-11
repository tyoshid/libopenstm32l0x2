/*
 * spi_sd.h
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of spi_sd.
 *
 * spi_sd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * spi_sd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with spi_sd.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Physical sector size (?) = logical sector size = block length */
#define SECTOR_SIZE			512

void delay_ms(int ms);
int sd_init(void);
int sd_read_sector(unsigned int sector, unsigned char *d);
void print_csd(unsigned char *d);
void print_cid(unsigned char *d);
void dump(unsigned char *d, int size);
int proc_mbr(unsigned char *d);
int proc_vbr(unsigned char *d);
int fat(unsigned char *d);
void command(void);
