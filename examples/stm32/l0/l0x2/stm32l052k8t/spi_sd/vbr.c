/*
 * vbr.c
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

/* Volume Boot Record */

struct fat_boot_sector {
	uint8_t ignored[3];
	uint8_t system_id[8];
	uint8_t sector_size[2];
	uint8_t sec_per_clus;
	uint16_t reserved;
	uint8_t fats;
	uint8_t dir_entries[2];
	uint8_t sectors[2];
	uint8_t media;
	uint16_t fat_length;
	uint16_t secs_track;
	uint16_t heads;
	uint32_t hidden;
	uint32_t total_sect;
	union {
		struct {
			uint8_t drive_number;
			uint8_t state;
			uint8_t signature;
			uint8_t vol_id[4];
			uint8_t vol_label[11];
			uint8_t fs_type[8];
		} fat16;
		struct {
			uint32_t length;
			uint16_t flags;
			uint8_t version[2];
			uint32_t root_cluster;
			uint16_t info_sector;
			uint16_t backup_boot;
			uint16_t reserved2[6];
			uint8_t drive_number;
			uint8_t state;
			uint8_t signature;
			uint8_t vol_id[4];
			uint8_t vol_label[11];
			uint8_t fs_type[8];
		} fat32;
	};
};

unsigned int fsinfo;		/* Logical sector number of FSinfo */
unsigned int root_dir;		/* Cluster number of root directory start */
unsigned int reserved_sector;	/* Count of reserved logical sectors */
unsigned int total_sector;	/* Total logical sector */
unsigned int fat_size;		/* Logical sectors per FAT */
unsigned int fat_num;		/* Number of FATs */
int cluster_size;		/* 1, 2, 4, 8, 16, 32, 64, and 128 */

int proc_vbr(unsigned char *d)
{
	struct fat_boot_sector *p;
	int i;
	
	puts("Boot sector:");
	if (*(d + 510) != 0x55 || *(d + 511) != 0xaa) {
		dump(d, SECTOR_SIZE);
		return -1;
	}
	p = (struct fat_boot_sector *)d;
	printf("\tOEM name = \"");
	for (i = 0; i < 8; i++)
		printf("%c", p->system_id[i]);
	printf("\"\n");
	printf("\tBytes per logical sector = %u\n",
	       p->sector_size[1] << 8 | p->sector_size[0]);
	printf("\tLogical sectors per cluster = %u\n", p->sec_per_clus);
	cluster_size = p->sec_per_clus;
	printf("\tCount of reserved logical sectors = %u\n", p->reserved);
	reserved_sector = p->reserved;
	printf("\tNumber of File Allocation Tables = %u\n", p->fats);
	fat_num = p->fats;
	if (p->dir_entries[1] << 8 | p->dir_entries[0])
		printf("\tMaximum number of root directory entries = %u\n",
		       p->dir_entries[1] << 8 | p->dir_entries[0]);
	if (p->sectors[1] << 8 | p->sectors[0])
		printf("\tTotal logical sectors = %u\n",
		       p->sectors[1] << 8 | p->sectors[0]);
	printf("\tMedia descriptor = 0x%02x\n", p->media);
	if (p->fat_length)
		printf("\tLogical sectors per File Allocation Table = %u\n",
		       p->fat_length);
	printf("\tPhysical secters per track = %u\n", p->secs_track);
	printf("\tNumber of heads = %u\n", p->heads);
	printf("\tCount of hidden sectors = %u\n", (unsigned int)p->hidden);
	printf("\tTotal logical sectors = %u\n", (unsigned int)p->total_sect);
	total_sector = p->total_sect;
	printf("\tLogical sectors per File Allocation Table = %u\n",
	       (unsigned int)p->fat32.length);
	fat_size = p->fat32.length;
	printf("\tDrive description / mirroring flags = 0x%04x\n",
	       p->fat32.flags);
	printf("\tVersion = %u.%u\n",
	       p->fat32.version[1], p->fat32.version[0]);
	printf("\tCluster number of root directory start = %u\n",
	       (unsigned int)p->fat32.root_cluster);
	root_dir = p->fat32.root_cluster;
	printf("\tLogical sector number of FS Information Sector = %u\n",
	       p->fat32.info_sector);
	fsinfo = p->fat32.info_sector;
	printf("\tFirst logical sector number of a copy of the three "
	       "boot sectors = %u\n", p->fat32.backup_boot);
	printf("\tPhysical drive number = %u\n", p->fat32.drive_number);
	if (p->fat32.state)
		printf("\tState = 0x%02x\n", p->fat32.state);
	printf("\tExtended boot signature = 0x%02x\n", p->fat32.signature);
	printf("\tVolume ID = %02x%02x-%02x%02x\n",
	       p->fat32.vol_id[3], p->fat32.vol_id[2], p->fat32.vol_id[1],
	       p->fat32.vol_id[0]);
	printf("\tPartition Volume Label = \"");
	for (i = 0; i < 11; i++)
		printf("%c", p->fat32.vol_label[i]);
	printf("\"\n");
	printf("\tFile system type = \"");
	for (i = 0; i < 8; i++)
		printf("%c", p->fat32.fs_type[i]);
	printf("\"\n");
	return 0;
}
