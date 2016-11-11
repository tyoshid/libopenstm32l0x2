/*
 * mbr.c
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

#include <stdio.h>
#include <stdint.h>

#include "spi_sd.h"

/* Master Boot record */

struct mbr_record {
	uint8_t boot_indicator;
	uint8_t start_head;
	uint8_t start_sector;
	uint8_t start_track;
	uint8_t os_type;
	uint8_t end_head;
	uint8_t end_sector;
	uint8_t end_track;
	uint8_t starting_lba[4];
	uint8_t size_in_lba[4];
};

struct legacy_mbr {
	uint8_t boot_code[440];
	uint32_t unique_mbr_signature;
	uint16_t unknown;
	struct mbr_record partition_record[4];
	uint16_t signature;
};

unsigned int boot_sector;	/* Block unit address of boot sector */

int proc_mbr(unsigned char *d)
{
	struct legacy_mbr *mbr;
	int i;
	struct mbr_record *p;
	unsigned int u;
	double w;

	puts("Master boot record:");
	mbr = (struct legacy_mbr *)d;
	if (mbr->signature != 0xaa55) {
		dump(d, SECTOR_SIZE);
		return -1;
	}
	puts("\tPartition Boot      Start        End    Sectors    Size Id "
	     "Type");
	for (i = 0; i < 4; i++) {
		p = &mbr->partition_record[i];
		if (p->os_type) {
			printf("\t%d         ", i + 1);
			printf("%c    ", p->boot_indicator ? '*' : ' ');
			printf("%10u ", ((p->start_sector & 0xc0) << 2 |
					 p->start_track) * 255 * 63 +
			       p->start_head * 63 +
			       (p->start_sector & 0x3f) - 1);
#if 0
			printf("%10u ", (unsigned int)
			       (p->starting_lba[3] << 24 |
				p->starting_lba[2] << 16 |
				p->starting_lba[1] << 8 |
				p->starting_lba[0]));
#endif
			printf("%10u ", ((p->end_sector & 0xc0) << 2 |
					 p->end_track) * 255 * 63 +
			       p->end_head * 63 + (p->end_sector & 0x3f) - 1);
			u = ((p->end_sector & 0xc0) << 2 | p->end_track) -
				((p->start_sector & 0xc0) << 2 |
				 p->start_track);
			u *= 255;
			u += p->end_head;
			u -= p->start_head;
			u *= 63;
			u += (p->end_sector & 0x3f);
			u -= (p->start_sector & 0x3f);
			u++;
			printf("%10u ", u);
#if 0
			printf("%10u ", (unsigned int)
			       (p->size_in_lba[3] << 24 |
				p->size_in_lba[2] << 16 |
				p->size_in_lba[1] << 8 |
				p->size_in_lba[0]));
#endif
			w = u;
			w = w * SECTOR_SIZE / 1024. / 1024. / 1024.;
			printf("%5.1lfGB ", w);
			printf("%2x ", p->os_type);
			switch (p->os_type) {
			case 0x0b:
				printf("W95 FAT32\n");
				boot_sector = ((p->start_sector & 0xc0) << 2 |
					       p->start_track) * 255 * 63 +
					p->start_head * 63 +
					(p->start_sector & 0x3f) - 1;
				break;
			default:
				printf("\n");
				break;
			}
		}
	}
	if (boot_sector == 0)
		return -1;
	return 0;
}
